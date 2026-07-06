// Fill out your copyright notice in the Description page of Project Settings.


#include "PlayerController/SGLobbyPlayerController.h"
#include "GameFramework/PlayerState.h"
#include "Blueprint/UserWidget.h"
#include "SoccerGame/Public/UI/SGLobbyWidget.h"
#include "GameMode/SGLobbyGameMode.h"
#include "PlayerState/SGLobbyPlayerState.h"

void ASGLobbyPlayerController::BeginPlay()
{
	Super::BeginPlay();
	
	if (IsLocalController() == false)
	{
		return; 
	}
	
	if (UIWidgetClass == nullptr)
	{
		UE_LOG(LogTemp, Error, TEXT("UIWidgetClass 없음."));
	}
	
	if (IsValid(UIWidgetClass) == true)
	{
		UIWidgetInstance = CreateWidget<UUserWidget>(this, UIWidgetClass); 
		if (IsValid(UIWidgetInstance) == true)
		{
			UIWidgetInstance->AddToViewport();
			
			FInputModeUIOnly Mode;
			Mode.SetWidgetToFocus(UIWidgetInstance->GetCachedWidget());
			SetInputMode(Mode);
			
			bShowMouseCursor = true;
		}
	}
}

void ASGLobbyPlayerController::SellectReady()
{
	ASGLobbyPlayerState* MyPlayerState = GetPlayerState<ASGLobbyPlayerState>();
	if (MyPlayerState)
	{
		// 내 현재 레디 상태를 반전(토글)시켜서 서버 RPC로 전송
		bool bTargetReady = !MyPlayerState->IsReady();
		Server_SetReady(bTargetReady);
	}
}

void ASGLobbyPlayerController::RequestChangeTeam(FGameplayTag NewTeam)
{
	// 서버 클라이언트가 서버에 요청을 보냄
	ServerRequestChangeTeam(NewTeam);
}

void ASGLobbyPlayerController::Client_UpdateLobbyUI(const TArray<FSGPlayerLobbyInfo>& InPlayerInfos)
{
	// 컨트롤러가 들고 있는 로비 위젯 인스턴스가 안전하게 존재하는지 확인
	if (UIWidgetInstance)
	{
		if (USGLobbyWidget* LobbyWidget = Cast<USGLobbyWidget>(UIWidgetInstance))
		{
			// 위젯에게 방 전체 인원의 최신 종합 데이터를 넘겨줍니다.
			LobbyWidget->SetPlayerInfos(InPlayerInfos);
            
			// 새로 배치
			LobbyWidget->RefreshLobby();
			LobbyWidget->UpdateReadyButtonText();
		}
	}
}

void ASGLobbyPlayerController::TimeUIUpdate(int32 NewTime)
{
	// 본인 클래스의 멤버 변수이므로 안전하게 접근 가능!
	if (UIWidgetInstance)
	{
		if (USGLobbyWidget* LobbyWidget = Cast<USGLobbyWidget>(UIWidgetInstance))
		{
			// 선택지 A 적용: 위젯의 카운트다운 전용 함수를 안전하게 호출
			LobbyWidget->UpdateCountdownText(NewTime);
            
			UE_LOG(LogTemp, Log, TEXT("[LobbyPC] 위젯 카운트다운 텍스트 업데이트 성공: %d초"), NewTime);
		}
	}
}

void ASGLobbyPlayerController::InitializeLocalPlayerLobbyUI()
{
	if (IsValid(UILobbyWidgetClass) == true)
	{
		UIWidgetInstance = CreateWidget<UUserWidget>(this, UILobbyWidgetClass); 
		if (IsValid(UIWidgetInstance) == true)
		{
			UIWidgetInstance->AddToViewport();

			FInputModeUIOnly Mode;
			Mode.SetWidgetToFocus(UIWidgetInstance->GetCachedWidget());
			SetInputMode(Mode);

			bShowMouseCursor = true;
		}
	}
	
	//현재 화면에 생성되어 있는 로비 위젯 인스턴스가 있는지 확인
	if (UIWidgetInstance)
	{
		if (USGLobbyWidget* LobbyWidget = Cast<USGLobbyWidget>(UIWidgetInstance))
		{
			// 2. 내 컴퓨터의 PlayerState(서버 초기화 데이터)를 긁어옵니다.
			if (ASGLobbyPlayerState* MyLobbyPS = GetPlayerState<ASGLobbyPlayerState>())
			{
				MyLobbyPS->CurrentTeamTag = FGameplayTag::RequestGameplayTag(FName("Team.Waiting"));
				// 위젯의 PlayerInfos에 집어넣을 내 정보 구조체 생성
				FSGPlayerLobbyInfo MyInfo;
                
				// 이름 채우기 (CustomPlayerName이 비어있으면 기본 엔진 이름)
				MyInfo.UserName = MyLobbyPS->CustomPlayerName.IsEmpty() ? 
								  MyLobbyPS->GetPlayerName():MyLobbyPS->CustomPlayerName;
                
				// 팀 태그 채우기 (기본값 Team.Waiting)
				MyInfo.TeamTag = MyLobbyPS->CurrentTeamTag;
                
				// 레디 상태 채우기 (false)
				MyInfo.bIsReady = MyLobbyPS->bIsReady;

				// 위젯의 PlayerInfos 배열에 내 데이터를 추가
				LobbyWidget->AddPlayerInfos(MyInfo);

				//  화면을 갱신
				LobbyWidget->RefreshLobby();
				LobbyWidget->UpdateReadyButtonText();
			}
		}
	}
}

void ASGLobbyPlayerController::ServerRequestChangeTeam_Implementation(FGameplayTag NewTeam)
{
	if (ASGLobbyGameMode* LobbyGM = Cast<ASGLobbyGameMode>(GetWorld()->GetAuthGameMode()))
	{
		LobbyGM->ProcessChangeTeamRequest(this, NewTeam);
	}
}

bool ASGLobbyPlayerController::ServerRequestChangeTeam_Validate(FGameplayTag NewTeamTag)
{
	return true;
}

void ASGLobbyPlayerController::Server_SetReady_Implementation(bool bNewReadyState)
{
	ASGLobbyPlayerState* SG_PlayerState = GetPlayerState<ASGLobbyPlayerState>();
    if (SG_PlayerState)
    {
       // 플레이어 상태 변경 (값이 변경되면 OnRep에 의해 자동으로 전체 UI 브로드캐스트가 일어남)
       SG_PlayerState->SetReadyState(bNewReadyState);
        
       // 재검사 요청
       ASGLobbyGameMode* TitleGM = Cast<ASGLobbyGameMode>(GetWorld()->GetAuthGameMode());
       if (TitleGM)
       {
          // 기존에 사용하시던 GameMode 측 레디 감지 함수 호출 유지
          TitleGM->OnPlayerReadyChanged();
       }
    }
}

bool ASGLobbyPlayerController::Server_SetReady_Validate(bool bNewReadyState)
{
	return true;
}
