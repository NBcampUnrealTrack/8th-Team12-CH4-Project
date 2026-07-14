// Fill out your copyright notice in the Description page of Project Settings.


#include "Multiplay/SGMultiplayGameInstance.h"

#include "OnlineSessionSettings.h"
#include "OnlineSubsystem.h"
#include "Online/OnlineSessionNames.h"


void USGMultiplayGameInstance::OnDestroySessionComplete(FName SessionName, bool bWasSuccessful)
{
	if (SessionInterface.IsValid())
	{
		SessionInterface->ClearOnDestroySessionCompleteDelegate_Handle(DestroySessionCompleteDelegateHandle);
	}

	// =========================================================================
	// ★ [화면 로그 추가] 세션 파괴 완료 알림 (주황색, 5초)
	// =========================================================================
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
	
	// Online Subsystem과 인터페이스 연결
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
	if (!SessionInterface.IsValid()) return;
	// 기존에 존재하던 세션이 있다면 먼저 제거하는 안전장치
	FNamedOnlineSession* ExistingSession = SessionInterface->GetNamedSession(NAME_GameSession);
	if (ExistingSession != nullptr)
	{
		SessionInterface->DestroySession(NAME_GameSession);
		return;
	}
	FOnlineSessionSettings SessionSettings;
    
	// 현재 작동 중인 서브시스템이 NULL인지 Steam인지 판별
	bool bIsLAN = (IOnlineSubsystem::Get()->GetSubsystemName() == "NULL");
	SessionSettings.bIsLANMatch = bIsLAN;
    
	SessionSettings.NumPublicConnections = 5; // 최대 인원수
	SessionSettings.bAllowJoinInProgress = true;
	SessionSettings.bShouldAdvertise = true;
	SessionSettings.bUsesPresence = true; // ◀ 스팀에서는 Presence(상태 정보) 기반 매칭이 필수입니다.

	// =========================================================================
	// ★ [스팀 전용 필수 추가] 스팀 로비 설정 활성화
	// LAN 환경이 아닐 때(즉, 스팀일 때) 로비 기능을 활성화해야 스팀 서버가 방을 중개해 줍니다.
	// =========================================================================
	if (!bIsLAN)
	{
		SessionSettings.bUseLobbiesIfAvailable = true;
		SessionSettings.bUseLobbiesVoiceChatIfAvailable = false; // 보이스챗 안 쓰면 false
	}

	// 검색 시 필터링할 커스텀 데이터 설정 (예: 맵 이름)
	SessionSettings.Set(SETTING_MAPNAME, FString("SG_LobbyLevel"), EOnlineDataAdvertisementType::ViaOnlineServiceAndPing);

	ULocalPlayer* LocalPlayer = GetFirstGamePlayer();
	if (LocalPlayer)
	{
		SessionInterface->CreateSession(*LocalPlayer->GetPreferredUniqueNetId(), NAME_GameSession, SessionSettings);
	}
	/*
	if (SessionInterface.IsValid())
	{
		// 기존에 남아있는 세션 제거
		auto ExistingSession = SessionInterface->GetNamedSession(NAME_GameSession);
		if (ExistingSession != nullptr)
		{
			// ◀ 기존 세션이 있다면 파괴 델리게이트를 먼저 걸고 파괴 시작!
			DestroySessionCompleteDelegateHandle = SessionInterface->AddOnDestroySessionCompleteDelegate_Handle(DestroySessionCompleteDelegate);
			SessionInterface->DestroySession(NAME_GameSession);
			return;
		}
		
		// 방 생성 끝나면 알려주는 델리게이트 등록
		CreateSessionCompleteDelegateHandle = SessionInterface->AddOnCreateSessionCompleteDelegate_Handle(CreateSessionCompleteDelegate);
		
		FOnlineSessionSettings SessionSettings;
		
		// 현재 활성화된 네트워크가 NULL이면 LAN모드(true)로, 스팀이면 인터넷 모드(false)로 자동 설정 
		//SessionSettings.bIsLANMatch = IOnlineSubsystem::Get()->GetSubsystemName() == "NULL" ? true : false;
		bool bIsLAN = (IOnlineSubsystem::Get()->GetSubsystemName() == "NULL");
		SessionSettings.bIsLANMatch = bIsLAN;
		
		if (GEngine)
		{
			GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Cyan, 
			   FString::Printf(TEXT("[Multiplay] 서버 생성 시작... (모드: %s)"), bIsLAN ? TEXT("LAN") : TEXT("스팀 인터넷")));
		}
		
		SessionSettings.bIsLANMatch = false;
		SessionSettings.NumPublicConnections = 6; // 최대 인원수 TODO: 나중에 변수 가져오기
		SessionSettings.bAllowJoinInProgress = true; // 게임 중 난입 허용 여부
		SessionSettings.bAllowJoinViaPresence = true; // 스팀 친구창 등으로 접속 허용
		SessionSettings.bShouldAdvertise = true; // 방이 검색되도록 허용
		SessionSettings.bUsesPresence = true; // 스팀의 Presence(현재 상태) 기능 사용
		SessionSettings.bUseLobbiesIfAvailable = true; 
		
		// ◀ 스팀 검색 정확도를 높이기 위한 커스텀 세팅 (나의 프로젝트 전용 방 식별용)
		SessionSettings.Settings.Add(SETTING_MAPNAME, FOnlineSessionSetting(FString("SG_LobbyLevel"), EOnlineDataAdvertisementType::ViaOnlineService));
		// 엔진에 세션 생성 명령
		const ULocalPlayer* LocalPlayer = GetWorld()->GetFirstLocalPlayerFromController();
		if (LocalPlayer != nullptr)
		{
			SessionInterface->CreateSession(*LocalPlayer->GetPreferredUniqueNetId(), NAME_GameSession, SessionSettings);	
		}
		else
		{
			// error log: 플레이어 정보를 불러오지 못함
			if (GEngine)
			{
				GEngine->AddOnScreenDebugMessage(-1, 10.f, FColor::Red, TEXT("[Multiplay] 에러: LocalPlayer를 찾을 수 없습니다!"));
			}
		}
	 */
}

void USGMultiplayGameInstance::FindServers()
{
	if (!SessionInterface.IsValid()) return;
	ULocalPlayer* LocalPlayer = GetFirstGamePlayer();
	if (!LocalPlayer) return;

	SessionSearch = MakeShareable(new FOnlineSessionSearch());
    
	bool bIsLAN = (IOnlineSubsystem::Get()->GetSubsystemName() == "NULL");
	SessionSearch->bIsLanQuery = bIsLAN;
    
	// =========================================================================
	// ★ [스팀 전용 필수 추가] 검색 범위 제한 및 Presence 필터
	// =========================================================================
	if (!bIsLAN)
	{
		// 스팀 마스터 서버에서 검색할 최대 방 개수 제한 (AppID 480 과부하 방지)
		SessionSearch->MaxSearchResults = 100; 
        
		// 스팀 로비/Presence 세션만 쿼리하도록 설정
		SessionSearch->QuerySettings.Set(SEARCH_PRESENCE, true, EOnlineComparisonOp::Equals);
		SessionSearch->QuerySettings.Set(SEARCH_LOBBIES, true, EOnlineComparisonOp::Equals);
	}
	else
	{
		SessionSearch->MaxSearchResults = 10000; // LAN 환경은 제한 해제
	}
    
	// 우리가 지정한 맵 이름("SG_LobbyLevel")인 방만 필터링해서 수집
	SessionSearch->QuerySettings.Set(SETTING_MAPNAME, FString("SG_LobbyLevel"), EOnlineComparisonOp::Equals);
    
	SessionInterface->FindSessions(*LocalPlayer->GetPreferredUniqueNetId(), SessionSearch.ToSharedRef());

	/*
	if (SessionInterface.IsValid())
	{
		FindSessionsCompleteDelegateHandle = SessionInterface->AddOnFindSessionsCompleteDelegate_Handle(FindSessionsCompleteDelegate);
		
		// 검색 바구니 세팅
		SessionSearch = MakeShareable(new FOnlineSessionSearch());
		SessionSearch->MaxSearchResults = 10000;
		
		// =========================================================================
		// ★ [수정] 검색 시에도 LAN환경인지 스팀인지 자동 판별하여 매칭 처리
		// =========================================================================
		bool bIsLAN = (IOnlineSubsystem::Get()->GetSubsystemName() == "NULL");
		SessionSearch->bIsLanQuery = bIsLAN;
		// =========================================================================
		
		if (GEngine)
		{
			GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Cyan, 
			   FString::Printf(TEXT("[Multiplay] 활성화된 서버 방 탐색 중... (%s)"), bIsLAN ? TEXT("LAN 주소") : TEXT("스팀 서비스")));
		}
       
		SessionSearch->QuerySettings.Set(SEARCH_PRESENCE, true, EOnlineComparisonOp::Equals);
       
		// =========================================================================
		// ★ [추가] 중요! 방을 찾을 때 우리가 등록한 고유 맵 이름 필터 조건 추가
		// 이 조건이 있어야 스팀의 수많은 가짜 방(AppID 480) 중에서 우리 방만 골라냅니다.
		// =========================================================================
		SessionSearch->QuerySettings.Set(SETTING_MAPNAME, FString("SG_LobbyLevel"), EOnlineComparisonOp::Equals);
		// =========================================================================
		//SessionSearch->bIsLanQuery = false;
		//SessionSearch->QuerySettings.Set(SEARCH_PRESENCE, true, EOnlineComparisonOp::Equals);
		
		// 검색 시작!
		const ULocalPlayer* LocalPlayer = GetWorld()->GetFirstLocalPlayerFromController();
		SessionInterface->FindSessions(*LocalPlayer->GetPreferredUniqueNetId(), SessionSearch.ToSharedRef());
	}
	 */
}

void USGMultiplayGameInstance::JoinServer(int32 SessionIndex)
{
	if (SessionInterface.IsValid() && SessionSearch.IsValid())
	{
		if (SessionSearch->SearchResults.IsValidIndex(SessionIndex))
		{
			JoinSessionCompleteDelegateHandle = SessionInterface->AddOnJoinSessionCompleteDelegate_Handle(JoinSessionCompleteDelegate);
			
			FString OwningUserName = SessionSearch->SearchResults[SessionIndex].Session.OwningUserName;
			int32 PingInMs = SessionSearch->SearchResults[SessionIndex].PingInMs;
			UE_LOG(LogTemp, Warning, TEXT("[Multiplay] %s 님이 개설한 방에 접속을 시도합니다. (핑: %d ms)"), *OwningUserName, PingInMs);
			if (GEngine)
			{
				GEngine->AddOnScreenDebugMessage(-1, 10.f, FColor::Cyan, 
				   FString::Printf(TEXT("[Multiplay] 접속 시도 중: Host=%s, Ping=%dms"), *OwningUserName, PingInMs));
			}
			// 접속 시도
			const ULocalPlayer* LocalPlayer = GetWorld()->GetFirstLocalPlayerFromController();
			SessionInterface->JoinSession(*LocalPlayer->GetPreferredUniqueNetId(), NAME_GameSession, SessionSearch->SearchResults[SessionIndex]);
		}
	}
}

void USGMultiplayGameInstance::OnCreateSessionComplete(FName Sessionname, bool bWasSuccessful)
{
	if (SessionInterface.IsValid())
	{
		// 델리게이트 해제
		SessionInterface->ClearOnCreateSessionCompleteDelegate_Handle(CreateSessionCompleteDelegateHandle);
	}
	
	
	if (bWasSuccessful)
	{
		UE_LOG(LogTemp, Warning, TEXT("[Multiplay] 방 생성 성공! 로비로 이동합니다."))
		if (GEngine)
		{
			GEngine->AddOnScreenDebugMessage(-1, 10.0f, FColor::Green, TEXT("[Multiplay] 세션 생성 완료! ServerTravel을 시도합니다."));
		}
		
		UWorld* World = GetWorld();
		if (World)
		{
			// 레벨 열기
			World->ServerTravel("SG_LobbyLevel?listen");
		}
	}
	else
	{
		if (GEngine)
		{
			GEngine->AddOnScreenDebugMessage(-1, 10.0f, FColor::Red, TEXT("[Multiplay] 세션 생성 실패!"));
		}
	}
}

void USGMultiplayGameInstance::OnFindSessionsComplete(bool bWasSuccessful)
{
	if (SessionInterface.IsValid())
	{
		SessionInterface->ClearOnFindSessionsCompleteDelegate_Handle(FindSessionsCompleteDelegateHandle);
	}
	
	if (bWasSuccessful && SessionSearch.IsValid())
	{
		int32 FoundCount = SessionSearch->SearchResults.Num();
		UE_LOG(LogTemp, Warning, TEXT("[Multiplay] 검색 완료! 찾은 방 갯수: %d"), FoundCount);
       
		if (GEngine)
		{
			GEngine->AddOnScreenDebugMessage(-1, 10.0f, FColor::Green, 
			   FString::Printf(TEXT("🔍 [Multiplay] 세션 검색 완료! 발견된 매치: %d개"), FoundCount));
		}
		
		// 검색된 방이 있고, 그 방의 핑과 정보가 유효한지 꼼꼼하게 교차 검증
		if (SessionSearch->SearchResults.Num() > 0 && SessionSearch->SearchResults[0].IsValid())
		{
			// ◀ [안정성 추가] 발견 즉시 접속하기보다, 서버가 완전히 열리도록 0.2초 정도만 살짝 대기 후 접속합니다.
			if (GEngine)
			{
				GEngine->AddOnScreenDebugMessage(-1, 2.0f, FColor::Orange, TEXT("⏳ 기존 서버가 있습니다. 안전한 진입을 위해 0.2초간 대기합니다..."));
			}
			
			FTimerHandle JoinDelayTimer;
			GetWorld()->GetTimerManager().SetTimer(JoinDelayTimer, FTimerDelegate::CreateUObject(this, &USGMultiplayGameInstance::JoinServer, 0), 0.2f, false);
		}
		else
		{
			UE_LOG(LogTemp, Warning, TEXT("[Multiplay] 빈 방이 없습니다. 직접 서버를 생성합니다."));
			if (GEngine)
			{
				GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Orange, TEXT("⚠ 진입 가능한 방이 없습니다. 직접 방장이 되어 방을 개설합니다!"));
			}
			CreateServer();
		}
	}
	else
	{
		// =========================================================================
		// ★ [화면 로그 추가] 검색 실패 시 예외 처리 화면 출력 (빨간색, 10초)
		// =========================================================================
		if (GEngine)
		{
			GEngine->AddOnScreenDebugMessage(-1, 10.0f, FColor::Red, TEXT("❌ [Multiplay] 세션 네트워크 검색 쿼리 실패!"));
		}
	}
	
}

void USGMultiplayGameInstance::OnJoinSessionComplete(FName SessionName, EOnJoinSessionCompleteResult::Type Result)
{
	if (!SessionInterface.IsValid()) return;

	if (Result == EOnJoinSessionCompleteResult::Success)
	{
		APlayerController* PC = GetFirstLocalPlayerController();
		if (PC)
		{
			FString ConnectString;
			// 스팀용 암호화된 연결 주소(또는 LAN IP주소)를 알아서 추출해 줍니다.
			if (SessionInterface->GetResolvedConnectString(SessionName, ConnectString))
			{
				// 클라이언트를 방장의 맵으로 안전하게 이동시킵니다.
				PC->ClientTravel(ConnectString, ETravelType::TRAVEL_Absolute);
			}
		}
	}
	/*
	if (SessionInterface.IsValid())
	{
		SessionInterface->ClearOnJoinSessionCompleteDelegate_Handle(JoinSessionCompleteDelegateHandle);
	}
	
	if (Result == EOnJoinSessionCompleteResult::Success)
	{
		UE_LOG(LogTemp, Warning, TEXT("[Multiplay] 방 접속 성공! 로딩을 시작합니다."));
		
		// 스팀 서버로부터 들어갈 방의 실제 네트워크 주소를 받기
		FString ConnectInfo;
		if (SessionInterface->GetResolvedConnectString(NAME_GameSession, ConnectInfo))
		{
			
			UE_LOG(LogTemp, Warning, TEXT("[Multiplay] 방 접속에 성공했습니다!"));
			UE_LOG(LogTemp, Warning, TEXT("[Multiplay] 대상 서버 연결 정보(Resolved Address): %s"), *ConnectInfo);
          
			if (GEngine)
			{
				// 인게임 화면에 녹색 글씨로 연결 주소를 15초간 띄워줍니다.
				GEngine->AddOnScreenDebugMessage(-1, 15.0f, FColor::Green, 
				   FString::Printf(TEXT("[Multiplay] 접속 완료! 주소: %s"), *ConnectInfo));
			}
			
			
			APlayerController* PlayerController = GetFirstLocalPlayerController(0);
			if (PlayerController)
			{
				PlayerController->ClientTravel(ConnectInfo, ETravelType::TRAVEL_Absolute);
			}
		}
		else
		{
			UE_LOG(LogTemp, Error, TEXT("[Multiplay] 세션 접속은 성공했으나, 연결 주소(ConnectString)를 가져오는데 실패했습니다."));
			if (GEngine)
			{
				GEngine->AddOnScreenDebugMessage(-1, 10.0f, FColor::Red, TEXT("[Multiplay] 에러: 연결 주소 획득 실패!"));
			}
		}
		
	}
	else
	{
		FString FailReason = TEXT("알 수 없음");
		switch (Result)
		{
		case EOnJoinSessionCompleteResult::SessionIsFull: 
			FailReason = TEXT("방이 가득 찼습니다 (Full)"); 
			break;
		case EOnJoinSessionCompleteResult::SessionDoesNotExist: 
			FailReason = TEXT("존재하지 않는 방입니다"); 
			break;
		case EOnJoinSessionCompleteResult::CouldNotRetrieveAddress: 
			FailReason = TEXT("주소를 불러올 수 없습니다"); 
			break;
		case EOnJoinSessionCompleteResult::AlreadyInSession: 
			FailReason = TEXT("이미 세션에 참가 중입니다"); 
			break;
		case EOnJoinSessionCompleteResult::UnknownError: 
			FailReason = TEXT("알 수 없는 에러"); 
			break;
		}

		
		UE_LOG(LogTemp, Error, TEXT("[Multiplay] 방 접속에 실패했습니다. 사유: %s"), *FailReason);
		if (GEngine)
		{
			GEngine->AddOnScreenDebugMessage(-1, 10.0f, FColor::Red, 
			   FString::Printf(TEXT("[Multiplay] 접속 실패: %s"), *FailReason));
		}
		// =========================================================================
	}
	 */
	
}
