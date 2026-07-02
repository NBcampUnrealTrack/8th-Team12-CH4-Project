// Fill out your copyright notice in the Description page of Project Settings.


#include "PlayerController/SGLobbyPlayerController.h"
#include "GameFramework/PlayerState.h"
#include "Blueprint/UserWidget.h"
#include "GameMode/SGLobbyGameMode.h"
#include "SoccerGame/Public/GameState/SGLobbyGameState.h"
#include "SoccerGame/Public/PlayerState/SGLobbyPlayerState.h"

void ASGLobbyPlayerController::BeginPlay()
{
	Super::BeginPlay();
	
	if (IsLocalController() == false)
	{
		return; 
	}
	
	if (UIWidgetClass == nullptr)
	{
		UE_LOG(LogTemp, Error, TEXT("UIWidgetClass가 비어있습니다! 디테일 패널을 확인하세요."));
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

void ASGLobbyPlayerController::ToggleReady()
{
	ASGLobbyGameState* MyPlayerState = GetPlayerState<ASGLobbyGameState>();
	if (MyPlayerState)
	{
		bool bTargetReady = !MyPlayerState->bIsReady;
		Server_SetReady(bTargetReady);
	}
}

void ASGLobbyPlayerController::RequestChangeTeam(ESGPlayerTeam NewTeam)
{
	// 서버 클라이언트가 서버에 요청을 보냄
	ServerRequestChangeTeam(NewTeam);
}

void ASGLobbyPlayerController::ServerRequestChangeTeam_Implementation(ESGPlayerTeam NewTeam)
{
	// [서버에서 실행됨] 
	// 2차 검증: 현재 게임 상태(예: 후반전 진행 중에는 팀 변경 불가 등)를 체크할 수 있음
    
	// 검증을 통과했다면, 내 PlayerState를 가져와서 값을 변경하라고 명령함
	if (ASGLobbyPlayerState* SGPlayerState = GetPlayerState<ASGLobbyPlayerState>())
	{
		// PlayerState에 있는 순수 변경 함수를 호출
		SGPlayerState->SetTeamInternal(NewTeam);
		
		// 로비GameMode 에서 모든 유저가 레디 했는지 검사함수 추가
	}
}

bool ASGLobbyPlayerController::ServerRequestChangeTeam_Validate(ESGPlayerTeam NewTeam)
{
	return true;
}

void ASGLobbyPlayerController::Server_SetReady_Implementation(bool bNewReadyState)
{
	ASGLobbyPlayerState* SG_PlayerState = GetPlayerState<ASGLobbyPlayerState>();
	if (SG_PlayerState)
	{
		// 1. 플레이어 상태 변경 (전광판 갱신)
		SG_PlayerState->SetReadyState(bNewReadyState);
        
		// 2. 심판(GameMode)에게 방 안의 전반적인 레디 상태를 재검사하라고 통보!
		ASGLobbyGameMode* TitleGM = Cast<ASGLobbyGameMode>(GetWorld()->GetAuthGameMode());
		if (TitleGM)
		{
			TitleGM->OnPlayerReadyChanged();
		}
	}
}

bool ASGLobbyPlayerController::Server_SetReady_Validate(bool bNewReadyState)
{
	return true;
}
