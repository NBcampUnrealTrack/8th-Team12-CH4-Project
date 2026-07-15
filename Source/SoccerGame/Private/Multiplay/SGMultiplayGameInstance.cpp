// Fill out your copyright notice in the Description page of Project Settings.


#include "Multiplay/SGMultiplayGameInstance.h"

#include "OnlineSessionSettings.h"
#include "OnlineSubsystem.h"
#include "Online/OnlineSessionNames.h"
#include "Interfaces/OnlineIdentityInterface.h"
#include "Kismet/GameplayStatics.h"


void USGMultiplayGameInstance::OnDestroySessionComplete(FName SessionName, bool bWasSuccessful)
{
	if (SessionInterface.IsValid())
	{
		SessionInterface->ClearOnDestroySessionCompleteDelegate_Handle(DestroySessionCompleteDelegateHandle);
	}
	if (GEngine)
	{
		GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Orange, TEXT("[Multiplay] 기존 세션을 파괴했습니다. 서버를 재생성합니다."));
	}
	// 파괴가 끝났으니 안전하게 다시 CreateServer를 호출 (이번엔 ExistingSession이 없으므로 통과됨)
	GetWorld()->GetTimerManager().SetTimerForNextTick(this, &USGMultiplayGameInstance::CreateServer);
}

USGMultiplayGameInstance::USGMultiplayGameInstance()
{
	// 델리게이트 바인딩
	CreateSessionCompleteDelegate = FOnCreateSessionCompleteDelegate::CreateUObject(this, &ThisClass::OnCreateSessionComplete);
	FindSessionsCompleteDelegate = FOnFindSessionsCompleteDelegate::CreateUObject(this, &ThisClass::OnFindSessionsComplete);
	JoinSessionCompleteDelegate = FOnJoinSessionCompleteDelegate::CreateUObject(this, &ThisClass::OnJoinSessionComplete);
	
	DestroySessionCompleteDelegate = FOnDestroySessionCompleteDelegate::CreateUObject(this, &ThisClass::OnDestroySessionComplete);
}


void USGMultiplayGameInstance::Init()
{
	Super::Init();
	IOnlineSubsystem* DefaultOSS = IOnlineSubsystem::Get();
	IOnlineSubsystem* SteamOSS = IOnlineSubsystem::Get(TEXT("Steam"));
	if (DefaultOSS)
	{
		SessionInterface = DefaultOSS->GetSessionInterface();
	}
	
	IOnlineSubsystem* Subsystem = IOnlineSubsystem::Get();
	if (Subsystem)
	{
		SessionInterface = Subsystem->GetSessionInterface();
		UE_LOG(LogTemp, Warning, TEXT("찾아낸 서브시스템: %s"), *Subsystem->GetSubsystemName().ToString())
		if (GEngine)
		{
			GEngine->AddOnScreenDebugMessage(-1, 10.0f, FColor::Cyan, 
			   FString::Printf(TEXT("[System] 온라인 서브시스템 활성화: %s"), *Subsystem->GetSubsystemName().ToString()));
		}
	}
	
	
}


void USGMultiplayGameInstance::CreateServer()
{
	if (!SessionInterface.IsValid())
	{
		UE_LOG(LogTemp, Error,
			TEXT("[Multiplay] SessionInterface Invalid"));
		return;
	}

	//------------------------------------------------------------
	// 기존 세션이 있으면 제거 후 다시 생성
	//------------------------------------------------------------

	if (FNamedOnlineSession* ExistingSession =
		SessionInterface->GetNamedSession(NAME_GameSession))
	{
		DestroySessionCompleteDelegateHandle =
			SessionInterface->AddOnDestroySessionCompleteDelegate_Handle(
				DestroySessionCompleteDelegate);

		const bool bDestroyStarted =
			SessionInterface->DestroySession(NAME_GameSession);

		UE_LOG(LogTemp, Warning,
			TEXT("[Multiplay] Existing session destroy started: %s"),
			bDestroyStarted ? TEXT("TRUE") : TEXT("FALSE"));

		if (!bDestroyStarted)
		{
			SessionInterface
				->ClearOnDestroySessionCompleteDelegate_Handle(
					DestroySessionCompleteDelegateHandle);
		}

		return;
	}

	//------------------------------------------------------------
	// 활성화된 Online Subsystem 확인
	//------------------------------------------------------------

	IOnlineSubsystem* OSS = IOnlineSubsystem::Get();

	if (!OSS)
	{
		UE_LOG(LogTemp, Error,
			TEXT("[Multiplay] OnlineSubsystem == nullptr"));
		return;
	}

	const bool bIsLAN =
		OSS->GetSubsystemName() == FName(TEXT("NULL"));

	UE_LOG(LogTemp, Warning,
		TEXT("[Multiplay] Subsystem=%s / Mode=%s"),
		*OSS->GetSubsystemName().ToString(),
		bIsLAN ? TEXT("LAN") : TEXT("STEAM"));

	//------------------------------------------------------------
	// LocalPlayer 확인
	//------------------------------------------------------------

	ULocalPlayer* LocalPlayer = GetFirstGamePlayer();

	if (!LocalPlayer)
	{
		UE_LOG(LogTemp, Error,
			TEXT("[Multiplay] LocalPlayer == nullptr"));

		if (GEngine)
		{
			GEngine->AddOnScreenDebugMessage(
				-1,
				10.0f,
				FColor::Red,
				TEXT("[Multiplay] LocalPlayer를 찾을 수 없습니다."));
		}

		return;
	}

	const FUniqueNetIdRepl PreferredNetId =
		LocalPlayer->GetPreferredUniqueNetId();

	if (!PreferredNetId.IsValid())
	{
		UE_LOG(LogTemp, Error,
			TEXT("[Multiplay] PreferredUniqueNetId Invalid"));
		return;
	}

	//------------------------------------------------------------
	// 세션 설정
	//------------------------------------------------------------

	FOnlineSessionSettings SessionSettings;

	SessionSettings.bIsLANMatch = bIsLAN;
	SessionSettings.NumPublicConnections = 6;
	
	SessionSettings.NumPrivateConnections = 0;

	SessionSettings.bShouldAdvertise = true;
	SessionSettings.bAllowJoinInProgress = true;
	
	//SessionSettings.bAllowInvites = true;
	SessionSettings.bAllowJoinViaPresence = true;
	
	//SessionSettings.bAllowJoinViaPresenceFriendsOnly = false;
	// Steam Lobby에서는 두 값을 동일하게 유지
	SessionSettings.bUsesPresence = !bIsLAN;
	SessionSettings.bUseLobbiesIfAvailable = !bIsLAN;

	// 실제 보이스 채팅을 사용하지 않는다면 false
	//SessionSettings.bUseLobbiesVoiceChatIfAvailable = false;

	SessionSettings.Set(
		SETTING_MAPNAME,
		FString(TEXT("SG_LobbyLevel")),
		EOnlineDataAdvertisementType::ViaOnlineService);

	//------------------------------------------------------------
	// CreateSession 완료 델리게이트 등록
	//------------------------------------------------------------

	CreateSessionCompleteDelegateHandle =
		SessionInterface->AddOnCreateSessionCompleteDelegate_Handle(
			CreateSessionCompleteDelegate);

	UE_LOG(LogTemp, Warning,
		TEXT("[Multiplay] Before CreateSession: LAN=%d Presence=%d Lobby=%d"),
		SessionSettings.bIsLANMatch,
		SessionSettings.bUsesPresence,
		SessionSettings.bUseLobbiesIfAvailable);

	const bool bCreateStarted =
		SessionInterface->CreateSession(
			*PreferredNetId,
			NAME_GameSession,
			SessionSettings);

	UE_LOG(LogTemp, Warning,
		TEXT("[Multiplay] CreateSession Return: %s"),
		bCreateStarted ? TEXT("TRUE") : TEXT("FALSE"));

	if (!bCreateStarted)
	{
		SessionInterface
			->ClearOnCreateSessionCompleteDelegate_Handle(
				CreateSessionCompleteDelegateHandle);

		if (GEngine)
		{
			GEngine->AddOnScreenDebugMessage(
				-1,
				10.0f,
				FColor::Red,
				TEXT("[Multiplay] CreateSession 시작 실패"));
		}
	}
}

void USGMultiplayGameInstance::FindServers()
{
	if (SessionInterface.IsValid())
	{
		ULocalPlayer* LocalPlayer = GetFirstGamePlayer();
		if (LocalPlayer == nullptr)
		{
			if (GEngine)
			{
				GEngine->AddOnScreenDebugMessage(-1, 10.f, FColor::Red, TEXT("[Multiplay] 에러: LocalPlayer를 찾을 수 없어 검색을 중단합니다."));
			}
			return;
		}
		
		FindSessionsCompleteDelegateHandle = SessionInterface->AddOnFindSessionsCompleteDelegate_Handle(FindSessionsCompleteDelegate);
		
		// 검색 바구니 세팅
		SessionSearch = MakeShareable(new FOnlineSessionSearch());
		SessionSearch->MaxSearchResults = 10000;
		
		// =========================================================================
		// ★ [수정] 검색 시에도 LAN환경인지 스팀인지 자동 판별하여 매칭 처리
		// =========================================================================
		IOnlineSubsystem* OSS = IOnlineSubsystem::Get();

		bool bIsLAN = true;

		if (OSS)
		{
			bIsLAN = (OSS->GetSubsystemName()=="NULL");
			UE_LOG(
				LogTemp,
				Warning,
				TEXT("[Multiplay] OnlineSubsystem: %s"),
				*OSS->GetSubsystemName().ToString()
			);
		}
		else
		{
			UE_LOG(
				LogTemp,
				Error,
				TEXT("[Multiplay] OnlineSubsystem == nullptr")
			);
			return;
		}
		SessionSearch->bIsLanQuery = bIsLAN;
		if (!bIsLAN)
		{
			SessionSearch->MaxSearchResults = 100; // 스팀 검색 개수 제한
			SessionSearch->QuerySettings.Set(SEARCH_LOBBIES,true,EOnlineComparisonOp::Equals);
		}
		else
		{
			SessionSearch->MaxSearchResults = 10000; // LAN 환경은 그대로 높게 유지
			SessionSearch->QuerySettings.Set(SEARCH_PRESENCE, true, EOnlineComparisonOp::Equals);
		}
		
		SessionSearch->QuerySettings.Set(SETTING_MAPNAME,FString(TEXT("SG_LobbyLevel")),
			EOnlineComparisonOp::Equals);
		
		const bool bFindStarted =SessionInterface->FindSessions(*LocalPlayer->GetPreferredUniqueNetId(),
				SessionSearch.ToSharedRef());
		
		UE_LOG(LogTemp,Warning,TEXT("[Multiplay] FindSessions Return: %s"),
			bFindStarted? TEXT("TRUE"): TEXT("FALSE"));

		if (!bFindStarted)
		{
			SessionInterface->ClearOnFindSessionsCompleteDelegate_Handle(
					FindSessionsCompleteDelegateHandle);
			UE_LOG(LogTemp,Error,TEXT("[Multiplay] FindSessions 시작 실패"));
		}
	}
	else
	{
		UE_LOG(
			LogTemp,
			Error,
			TEXT("[Multiplay] FindServers 실패: SessionInterface Invalid")
		);
	}
}

void USGMultiplayGameInstance::JoinServer(int32 SessionIndex)
{
	
	UE_LOG(LogTemp, Warning, TEXT("=========================================="));
	UE_LOG(LogTemp, Warning, TEXT("JoinServer()"));

	if (!SessionInterface.IsValid())
	{
		UE_LOG(LogTemp, Error, TEXT("SessionInterface Invalid"));
		return;
	}

	if (!SessionSearch.IsValid())
	{
		UE_LOG(LogTemp, Error, TEXT("SessionSearch Invalid"));
		return;
	}

	if (!SessionSearch->SearchResults.IsValidIndex(SessionIndex))
	{
		UE_LOG(LogTemp, Error,
			TEXT("Invalid Session Index : %d"),
			SessionIndex);
		return;
	}
	

	ULocalPlayer* LocalPlayer = GetFirstGamePlayer();

	if (LocalPlayer == nullptr)
	{
		UE_LOG(LogTemp, Error, TEXT("LocalPlayer == nullptr"));
		return;
	}
	const FOnlineSessionSearchResult SearchResult =
	  SessionSearch->SearchResults[SessionIndex];
	
	UE_LOG(LogTemp, Warning,
		TEXT("Search Presence=%d"),
		SearchResult.Session.SessionSettings.bUsesPresence);

	UE_LOG(LogTemp, Warning,
		TEXT("Search Lobby=%d"),
		SearchResult.Session.SessionSettings.bUseLobbiesIfAvailable);

	UE_LOG(LogTemp, Warning,
		TEXT("Host : %s"),
		*SearchResult.Session.OwningUserName);
	
	UE_LOG(LogTemp, Warning,
		TEXT("Ping : %d"),
		SearchResult.PingInMs);

	UE_LOG(LogTemp, Warning,
		TEXT("Open Connections : %d"),
		SearchResult.Session.NumOpenPublicConnections);

	UE_LOG(LogTemp, Warning,
		TEXT("Max Connections : %d"),
		SearchResult.Session.SessionSettings.NumPublicConnections);

	UE_LOG(LogTemp, Warning,
		TEXT("SessionInfo Valid : %s"),
		SearchResult.Session.SessionInfo.IsValid()
			? TEXT("YES")
			: TEXT("NO"));

	FString MapName;
	
	const bool bHasMapName =
		SearchResult.Session.SessionSettings.Get(
			SETTING_MAPNAME,
			MapName);

	UE_LOG(LogTemp, Warning,
		TEXT("Map : %s"),
		*MapName);
	

	//--------------------------------------------------------
	// Join Delegate 등록
	//--------------------------------------------------------
	if (JoinSessionCompleteDelegateHandle.IsValid())
	{
		SessionInterface->ClearOnJoinSessionCompleteDelegate_Handle(
			JoinSessionCompleteDelegateHandle);
		
		JoinSessionCompleteDelegateHandle.Reset();

	}
	JoinSessionCompleteDelegateHandle =
			SessionInterface->AddOnJoinSessionCompleteDelegate_Handle(
				JoinSessionCompleteDelegate);
	if (GEngine)
	{
		GEngine->AddOnScreenDebugMessage(
			-1,
			5.f,
			FColor::Cyan,
			FString::Printf(
				TEXT("[Multiplay] Join Try : %s"),
				*SearchResult.Session.OwningUserName));
	}

	//--------------------------------------------------------
	// Join Session
	//--------------------------------------------------------

	bool bJoinResult =
		SessionInterface->JoinSession(
			*LocalPlayer->GetPreferredUniqueNetId(),
			NAME_GameSession,
			SearchResult);

	UE_LOG(LogTemp, Warning,
		TEXT("JoinSession() Return : %s"),
		bJoinResult ? TEXT("TRUE") : TEXT("FALSE"));

	if (!bJoinResult)
	{
		UE_LOG(LogTemp, Error,
			TEXT("JoinSession() returned FALSE"));
		// 요청 시작 실패 시 완료 콜백이 오지 않을 수 있으므로
		// 등록한 델리게이트를 직접 정리합니다.
		SessionInterface->ClearOnJoinSessionCompleteDelegate_Handle(
			JoinSessionCompleteDelegateHandle);

		if (GEngine)
		{
			GEngine->AddOnScreenDebugMessage(
				-1,
				10.f,
				FColor::Red,
				TEXT("[Multiplay] JoinSession() Failed"));
		}
	}

	UE_LOG(LogTemp, Warning, TEXT("=========================================="));
}

void USGMultiplayGameInstance::OnCreateSessionComplete(FName Sessionname, bool bWasSuccessful)
{
	if (SessionInterface.IsValid())
	{
		SessionInterface->ClearOnCreateSessionCompleteDelegate_Handle(
			CreateSessionCompleteDelegateHandle);
	}

	if (!bWasSuccessful)
	{
		UE_LOG(
			LogTemp,
			Error,
			TEXT("[Multiplay] CreateSession 실패: %s"),
			*Sessionname.ToString());

		if (GEngine)
		{
			GEngine->AddOnScreenDebugMessage(
				-1,
				10.0f,
				FColor::Red,
				TEXT("[Multiplay] 세션 생성 실패"));
		}

		return;
	}

	UE_LOG(
		LogTemp,
		Warning,
		TEXT("[Multiplay] 세션 생성 성공"));

	if (SessionInterface.IsValid())
	{
		if (FNamedOnlineSession* NamedSession =
			SessionInterface->GetNamedSession(NAME_GameSession))
		{
			UE_LOG(
				LogTemp,
				Warning,
				TEXT("[Multiplay] Host Presence=%s / Lobby=%s"),
				NamedSession->SessionSettings.bUsesPresence
					? TEXT("TRUE")
					: TEXT("FALSE"),
				NamedSession->SessionSettings.bUseLobbiesIfAvailable
					? TEXT("TRUE")
					: TEXT("FALSE"));
		}
	}

	UWorld* World = GetWorld();

	if (!World)
	{
		UE_LOG(
			LogTemp,
			Error,
			TEXT("[Multiplay] GetWorld() == nullptr"));

		return;
	}
	

	//const FName LobbyMapName =TEXT("Game/SoccerGame/Maps/System/SG_LobbyLevel");
	UE_LOG(LogTemp, Warning,
	TEXT("[Multiplay] OpenLevel Start"));
	World->ServerTravel(TEXT("/Game/SoccerGame/Maps/System/SG_LobbyLevel?listen"));

}

void USGMultiplayGameInstance::OnFindSessionsComplete(bool bWasSuccessful)
{
	if (SessionInterface.IsValid())
	{
		SessionInterface->ClearOnFindSessionsCompleteDelegate_Handle(
			FindSessionsCompleteDelegateHandle);
	}
	

	UE_LOG(LogTemp, Warning, TEXT("=========================================="));
	UE_LOG(LogTemp, Warning, TEXT("OnFindSessionsComplete"));
	UE_LOG(LogTemp, Warning, TEXT("Search Success : %s"),
		bWasSuccessful ? TEXT("TRUE") : TEXT("FALSE"));

	if (!bWasSuccessful)
	{
		UE_LOG(LogTemp, Error, TEXT("FindSessions Failed"));

		if (GEngine)
		{
			GEngine->AddOnScreenDebugMessage(-1,10.f,FColor::Red,
				TEXT("[Multiplay] FindSessions Failed"));
		}
		return;
	}

	if (!SessionSearch.IsValid())
	{
		UE_LOG(LogTemp, Error, TEXT("SessionSearch Invalid"));
		return;
	}

	int32 FoundCount = SessionSearch->SearchResults.Num();

	UE_LOG(LogTemp, Warning,
		TEXT("Search Result Count : %d"),
		FoundCount);

	if (GEngine)
	{
		GEngine->AddOnScreenDebugMessage(
			-1,5.f,FColor::Green,
			FString::Printf(TEXT("[Multiplay] Found %d Sessions"), FoundCount));
	}

	FString MySteamName;

	if (ULocalPlayer* LocalPlayer = GetFirstGamePlayer())
	{
		MySteamName = LocalPlayer->GetNickname();

		UE_LOG(LogTemp, Warning,
			TEXT("My Steam Name : %s"),
			*MySteamName);
	}

	bool bFoundValidSession = false;
	int32 TargetSessionIndex = INDEX_NONE;

	//---------------------------------------------------------
	// 모든 검색 결과 출력
	//---------------------------------------------------------

	for (int32 i = 0; i < SessionSearch->SearchResults.Num(); ++i)
	{
		const FOnlineSessionSearchResult& Result =
			SessionSearch->SearchResults[i];

		UE_LOG(LogTemp, Warning, TEXT("--------------------------------"));

		if (!Result.IsValid())
		{
			UE_LOG(LogTemp, Warning,
				TEXT("[%d] Invalid Search Result"),i);
			continue;
		}
		UE_LOG(LogTemp, Warning,TEXT("Presence : %s"),
			Result.Session.SessionSettings.bUsesPresence? TEXT("TRUE"): TEXT("FALSE"));

		UE_LOG(LogTemp, Warning,
			TEXT("Lobby : %s"),
			Result.Session.SessionSettings.bUseLobbiesIfAvailable
				? TEXT("TRUE")
				: TEXT("FALSE"));

		FString MapName;
		
		const bool bHasMapName =
			Result.Session.SessionSettings.Get(
				SETTING_MAPNAME,
				MapName);
		Result.Session.SessionSettings.Get(
			SETTING_MAPNAME,
			MapName);

		UE_LOG(LogTemp, Warning,
			TEXT("[%d] Host : %s"),
			i,
			*Result.Session.OwningUserName);

		UE_LOG(LogTemp, Warning,
			TEXT("[%d] Ping : %d"),
			i,
			Result.PingInMs);

		UE_LOG(LogTemp, Warning,
			TEXT("[%d] Map : %s"),
			i,
			*MapName);

		UE_LOG(LogTemp, Warning,
			TEXT("[%d] Open Connection : %d"),
			i,
			Result.Session.NumOpenPublicConnections);

		UE_LOG(LogTemp, Warning,
			TEXT("[%d] Max Connection : %d"),
			i,
			Result.Session.SessionSettings.NumPublicConnections);

		UE_LOG(LogTemp, Warning,
			TEXT("[%d] SessionInfo Valid : %s"),
			i,
			Result.Session.SessionInfo.IsValid()
			? TEXT("YES")
			: TEXT("NO"));

		//-----------------------------------------------------
		// 내 세션 제외
		//-----------------------------------------------------

		if (!MySteamName.IsEmpty() &&
			Result.Session.OwningUserName == MySteamName)
		{
			UE_LOG(LogTemp, Warning,
				TEXT("[%d] Skip My Session"),
				i);
			continue;
		}

		// ========================================================
		// ★ MAPNAME 필터를 FindServers()에 넣었다면
		// ★ 여기의 맵 체크는 중복 검사입니다.
		// ★ 디버깅 단계에서는 유지하는 것이 안전합니다.
		// ========================================================


		if (!bHasMapName  ||MapName != TEXT("SG_LobbyLevel"))
		{
			UE_LOG(LogTemp, Warning,
				TEXT("[%d] Skip (Different Map)"),
				i);
			continue;
		}

		//-----------------------------------------------------
		// 빈 자리 체크
		//-----------------------------------------------------

		if (Result.Session.NumOpenPublicConnections <= 0)
		{
			UE_LOG(LogTemp, Warning,
				TEXT("[%d] Skip (No Empty Slot)"),
				i);
			continue;
		}

		//-----------------------------------------------------
		// SessionInfo 체크
		//-----------------------------------------------------

		if (!Result.Session.SessionInfo.IsValid())
		{
			UE_LOG(LogTemp, Warning,
				TEXT("[%d] Skip (SessionInfo Invalid)"),
				i);
			continue;
		}

		//-----------------------------------------------------
		// 조건 만족
		//-----------------------------------------------------

		UE_LOG(LogTemp, Warning,
			TEXT("[%d] Selected Session"),
			i);

		bFoundValidSession = true;
		TargetSessionIndex = i;
		break;
	}

	//---------------------------------------------------------
	// Join
	//---------------------------------------------------------

	if (bFoundValidSession)
	{
		UE_LOG(LogTemp, Warning,
			TEXT("Join Target Index : %d"),
			TargetSessionIndex);

		if (GEngine)
		{
			GEngine->AddOnScreenDebugMessage(
				-1,
				5.f,
				FColor::Yellow,
				TEXT("[Multiplay] Valid Session Found"));
		}
		JoinServer(TargetSessionIndex);

		//FTimerHandle TimerHandle;
		//GetWorld()->GetTimerManager().SetTimer(
		//	TimerHandle,
		//	FTimerDelegate::CreateUObject(
		//		this,
		//		&USGMultiplayGameInstance::JoinServer,
		//		TargetSessionIndex),
		//	2.0f,      // 기존 0.2 -> 2초
		//	false);
	}
	else
	{
		UE_LOG(LogTemp, Warning,
			TEXT("No Valid Session -> CreateServer"));

		if (GEngine)
		{
			GEngine->AddOnScreenDebugMessage(
				-1,
				5.f,
				FColor::Orange,
				TEXT("[Multiplay] No Valid Session. Create Server."));
		}

		CreateServer();
	}

	UE_LOG(LogTemp, Warning, TEXT("=========================================="));
}


void USGMultiplayGameInstance::OnJoinSessionComplete(FName SessionName, EOnJoinSessionCompleteResult::Type Result)
{
	if (SessionInterface.IsValid())
	{
		SessionInterface->ClearOnJoinSessionCompleteDelegate_Handle(
			JoinSessionCompleteDelegateHandle);
	}

	UE_LOG(LogTemp, Warning, TEXT("=========================================="));
	UE_LOG(LogTemp, Warning, TEXT("OnJoinSessionComplete()"));

	FString ResultString;

	switch (Result)
	{
	case EOnJoinSessionCompleteResult::Success:
		ResultString = TEXT("Success");
		break;

	case EOnJoinSessionCompleteResult::SessionIsFull:
		ResultString = TEXT("SessionIsFull");
		break;

	case EOnJoinSessionCompleteResult::SessionDoesNotExist:
		ResultString = TEXT("SessionDoesNotExist");
		break;

	case EOnJoinSessionCompleteResult::CouldNotRetrieveAddress:
		ResultString = TEXT("CouldNotRetrieveAddress");
		break;

	case EOnJoinSessionCompleteResult::AlreadyInSession:
		ResultString = TEXT("AlreadyInSession");
		break;

	case EOnJoinSessionCompleteResult::UnknownError:
		ResultString = TEXT("UnknownError");
		break;

	default:
		ResultString = TEXT("Unknown");
		break;
	}

	UE_LOG(LogTemp, Warning,
		TEXT("Join Result : %s"),
		*ResultString);

	if (Result != EOnJoinSessionCompleteResult::Success)
	{
		UE_LOG(
			LogTemp,
			Error,
			TEXT("Join Failed : %s"),
			*ResultString);
		return;
	}
	if (!SessionInterface.IsValid())
	{
		UE_LOG(
			LogTemp,
			Error,
			TEXT("[Multiplay] SessionInterface Invalid after Join"));

		return;
	}
	
		FString ConnectInfo;

		const bool bResolved =
			SessionInterface->GetResolvedConnectString(
				NAME_GameSession,
				ConnectInfo);

		UE_LOG(LogTemp, Warning,
			TEXT("GetResolvedConnectString : %s"),
			bResolved ? TEXT("SUCCESS") : TEXT("FAIL"));

		if (!bResolved)
		{
			UE_LOG(LogTemp, Error,
				TEXT("ConnectString 획득 실패"));

			if (GEngine)
			{
				GEngine->AddOnScreenDebugMessage(
					-1,
					10.f,
					FColor::Red,
					TEXT("[Multiplay] ConnectString 획득 실패"));
			}

			return;
		}

		UE_LOG(LogTemp, Warning,
			TEXT("ConnectString : %s"),
			*ConnectInfo);

		APlayerController* PlayerController =
			GetFirstLocalPlayerController();

		if (PlayerController == nullptr)
		{
			UE_LOG(LogTemp, Error,
				TEXT("PlayerController == nullptr"));

			return;
		}

	
		PlayerController->ClientTravel(
			ConnectInfo,
			ETravelType::TRAVEL_Absolute);
}
