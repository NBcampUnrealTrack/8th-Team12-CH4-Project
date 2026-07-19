// Fill out your copyright notice in the Description page of Project Settings.


#include "Multiplay/SGMultiplayGameInstance.h"

#include "OnlineSessionSettings.h"
#include "OnlineSubsystem.h"
#include "Components/SplineComponent.h"
#include "Online/OnlineSessionNames.h"
#include "Interfaces/OnlineIdentityInterface.h"
#include "Kismet/GameplayStatics.h"


void USGMultiplayGameInstance::OnDestroySessionComplete(FName SessionName, bool bWasSuccessful)
{
	if (SessionInterface.IsValid())
	{
		SessionInterface->ClearOnDestroySessionCompleteDelegate_Handle(DestroySessionCompleteDelegateHandle);
	}
	DestroySessionCompleteDelegateHandle.Reset();
	const bool bShouldReturnToMainMenu =bReturnToMainMenuAfterDestroy;
	bReturnToMainMenuAfterDestroy = false;
	if (bShouldReturnToMainMenu)
	{
		OpenMainMenu();
		return;
	}

	if (!bWasSuccessful)
	{
		return;
	}

	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().SetTimerForNextTick(this,&USGMultiplayGameInstance::CreateServer);
	}
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
		UE_LOG(LogTemp, Warning, TEXT("찾아낸 서브시스템: %s"), *Subsystem->GetSubsystemName().ToString());
		if (GEngine)
		{
			GEngine->AddOnScreenDebugMessage(-1, 10.0f, FColor::Cyan, 
			   FString::Printf(TEXT("[System] 온라인 서브시스템 활성화: %s")
			   	, *Subsystem->GetSubsystemName().ToString()));
		}
	}
	
	
}


void USGMultiplayGameInstance::CreateServer()
{
	if (!SessionInterface.IsValid())
	{
		return;
	}

	if (FNamedOnlineSession* ExistingSession =SessionInterface->GetNamedSession(NAME_GameSession))
	{
		bReturnToMainMenuAfterDestroy = false;
		
		DestroySessionCompleteDelegateHandle =
			SessionInterface->AddOnDestroySessionCompleteDelegate_Handle(DestroySessionCompleteDelegate);

		const bool bDestroyStarted =
			SessionInterface->DestroySession(NAME_GameSession);

		if (!bDestroyStarted)
		{
			SessionInterface->ClearOnDestroySessionCompleteDelegate_Handle(DestroySessionCompleteDelegateHandle);
		}

		return;
	}

	IOnlineSubsystem* OSS = IOnlineSubsystem::Get();

	if (!OSS)
	{
		return;
	}

	const bool bIsLAN =OSS->GetSubsystemName() == FName(TEXT("NULL"));
	ULocalPlayer* LocalPlayer = GetFirstGamePlayer();

	if (!LocalPlayer)
	{
		return;
	}

	const FUniqueNetIdRepl PreferredNetId =LocalPlayer->GetPreferredUniqueNetId();

	if (!PreferredNetId.IsValid())
	{
		return;
	}

	FOnlineSessionSettings SessionSettings;
	SessionSettings.bIsLANMatch = bIsLAN;
	SessionSettings.NumPublicConnections = 6;
	
	SessionSettings.NumPrivateConnections = 0;

	SessionSettings.bShouldAdvertise = true;
	SessionSettings.bAllowJoinInProgress = true;
	
	SessionSettings.bAllowJoinViaPresence = true;
	
	//SessionSettings.bAllowJoinViaPresenceFriendsOnly = false;
	// Steam Lobby에서는 두 값을 동일하게 유지
	SessionSettings.bUsesPresence = !bIsLAN;
	SessionSettings.bUseLobbiesIfAvailable = !bIsLAN;

	SessionSettings.Set(SETTING_MAPNAME,FString(TEXT("SG_LobbyLevel")),EOnlineDataAdvertisementType::ViaOnlineService);

	CreateSessionCompleteDelegateHandle =SessionInterface->AddOnCreateSessionCompleteDelegate_Handle(CreateSessionCompleteDelegate);

	const bool bCreateStarted =SessionInterface->CreateSession(*PreferredNetId,NAME_GameSession,SessionSettings);


	if (!bCreateStarted)
	{
		SessionInterface->ClearOnCreateSessionCompleteDelegate_Handle(CreateSessionCompleteDelegateHandle);
		bReturnToMainMenuAfterDestroy = false;
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
	FOnlineSessionSearchResult SearchResult =
	  SessionSearch->SearchResults[SessionIndex];
	
	
	// Steam JoinSession은 두 값이 같아야 함
	SearchResult.Session.SessionSettings.bUseLobbiesIfAvailable =
		SearchResult.Session.SessionSettings.bUsesPresence;

	UE_LOG(
		LogTemp,
		Warning,
		TEXT("[Join Fix] Presence=%d / Lobby=%d"),
		SearchResult.Session.SessionSettings.bUsesPresence,
		SearchResult.Session.SessionSettings.bUseLobbiesIfAvailable);
	
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

void USGMultiplayGameInstance::LeaveSessionAndReturnToMainMenu()
{
	if (!SessionInterface.IsValid())
	{
		OpenMainMenu();
		return;
	}
	if (SessionInterface->GetNamedSession(NAME_GameSession) == nullptr)
	{
		OpenMainMenu();
		return;
	}
	
	bReturnToMainMenuAfterDestroy = true;

	if (DestroySessionCompleteDelegateHandle.IsValid())
	{
		SessionInterface->ClearOnDestroySessionCompleteDelegate_Handle(DestroySessionCompleteDelegateHandle);

		DestroySessionCompleteDelegateHandle.Reset();
	}

	DestroySessionCompleteDelegateHandle =SessionInterface->AddOnDestroySessionCompleteDelegate_Handle(DestroySessionCompleteDelegate);

	const bool bDestroyStarted =SessionInterface->DestroySession(NAME_GameSession);

	if (!bDestroyStarted)
	{
		SessionInterface->ClearOnDestroySessionCompleteDelegate_Handle(DestroySessionCompleteDelegateHandle);
		DestroySessionCompleteDelegateHandle.Reset();
		bReturnToMainMenuAfterDestroy = false;
		OpenMainMenu();
	}
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
		return;
	}

	UWorld* World = GetWorld();

	if (!World)
	{
		return;
	}
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

void USGMultiplayGameInstance::OpenMainMenu()
{
	UWorld* World = GetWorld();

	if (!IsValid(World))
	{
		return;
	}

	UGameplayStatics::OpenLevel(World,FName(TEXT("/Game/SoccerGame/Maps/System/SG_MainMenu")));
}
