// Fill out your copyright notice in the Description page of Project Settings.


#include "Multiplay/SGMultiplayGameInstance.h"

#include "OnlineSessionSettings.h"
#include "OnlineSubsystem.h"


USGMultiplayGameInstance::USGMultiplayGameInstance()
{
	// 콜백 델리게이트와 OnCreateSessionComplete 연결
	CreateSessionCompleteDelegate = FOnCreateSessionCompleteDelegate::CreateUObject(this, &ThisClass::OnCreateSessionComplete);
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
		// 방 생성 끝나면 알려주는 델리게이트 등록
		CreateSessionCompleteDelegateHandle = SessionInterface->AddOnCreateSessionCompleteDelegate_Handle(CreateSessionCompleteDelegate);
		
		FOnlineSessionSettings SessionSettings;
		SessionSettings.bIsLANMatch = false; // 스팀(인터넷) 통신 사용
		SessionSettings.NumPublicConnections = 6; // 최대 인원수 TODO: 나중에 변수 가져오기
		SessionSettings.bAllowJoinInProgress = false; // 게임 중 난입 허용 여부
		SessionSettings.bAllowJoinViaPresence = true; // 스팀 친구창 등으로 접속 허용
		SessionSettings.bShouldAdvertise = true; // 방이 검색되도록 허용
		SessionSettings.bUsesPresence = true; // 스팀의 Presence(현재 상태) 기능 사용
		
		// 엔진에 세션 생성 명령
		const ULocalPlayer* LocalPlayer = GetWorld()->GetFirstLocalPlayerFromController();
		SessionInterface->CreateSession(*LocalPlayer->GetPreferredUniqueNetId(), NAME_GameSession, SessionSettings);
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
		
		UWorld* World = GetWorld();
		if (World)
		{
			// 레벨 열기
			World->ServerTravel("/Game/SoccerGame/Maps/SG_LobbyLevel?listen");
		}
	}
}
