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

	// 파괴가 끝났으니 안전하게 다시 CreateServer를 호출 (이번엔 ExistingSession이 없으므로 통과됨)
	CreateServer();
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
	}
	
}


void USGMultiplayGameInstance::CreateServer()
{
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
	}
}

void USGMultiplayGameInstance::FindServers()
{
	if (SessionInterface.IsValid())
	{
		FindSessionsCompleteDelegateHandle = SessionInterface->AddOnFindSessionsCompleteDelegate_Handle(FindSessionsCompleteDelegate);
		
		// 검색 바구니 세팅
		SessionSearch = MakeShareable(new FOnlineSessionSearch());
		SessionSearch->MaxSearchResults = 10000;
		SessionSearch->bIsLanQuery = false;
		SessionSearch->QuerySettings.Set(SEARCH_PRESENCE, true, EOnlineComparisonOp::Equals);
		
		// 검색 시작!
		const ULocalPlayer* LocalPlayer = GetWorld()->GetFirstLocalPlayerFromController();
		SessionInterface->FindSessions(*LocalPlayer->GetPreferredUniqueNetId(), SessionSearch.ToSharedRef());
	}
}

void USGMultiplayGameInstance::JoinServer(int32 SessionIndex)
{
	if (SessionInterface.IsValid() && SessionSearch.IsValid())
	{
		if (SessionSearch->SearchResults.IsValidIndex(SessionIndex))
		{
			JoinSessionCompleteDelegateHandle = SessionInterface->AddOnJoinSessionCompleteDelegate_Handle(JoinSessionCompleteDelegate);
			
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
		if (GEngine)
		{
			GEngine->AddOnScreenDebugMessage(-1, 10.0f, FColor::Green, TEXT("[Multiplay] 세션 생성 완료! ServerTravel을 시도합니다."));
		}
		UE_LOG(LogTemp, Warning, TEXT("[Multiplay] 방 생성 성공! 로비로 이동합니다."))
		
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
		UE_LOG(LogTemp, Warning, TEXT("[Multiplay] 검색 완료! 찾은 방 갯수: %d"), SessionSearch->SearchResults.Num());
		
		if (SessionSearch->SearchResults.Num() > 0)
		{
			// 0번 서버에 바로 접속 (자동 접속 시스템)
			JoinServer(0);
		}
		else
		{
			UE_LOG(LogTemp, Warning, TEXT("[Multiplay] 빈 방이 없습니다. 직접 서버를 생성합니다."));
			CreateServer();
		}
	}
	
}

void USGMultiplayGameInstance::OnJoinSessionComplete(FName SessionName, EOnJoinSessionCompleteResult::Type Result)
{
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
			APlayerController* PlayerController = GetFirstLocalPlayerController(0);
			if (PlayerController)
			{
				PlayerController->ClientTravel(ConnectInfo, ETravelType::TRAVEL_Absolute);
			}
		}
	}
}
