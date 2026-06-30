// Fill out your copyright notice in the Description page of Project Settings.


#include "PlayerController/SGLobbyPlayerController.h"
#include "GameFramework/PlayerState.h"
#include "Blueprint/UserWidget.h"
#include "SoccerGame/Public/GameState/SGLobbyGameState.h"

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
		else
		UE_LOG(LogTemp, Error, TEXT("2"));
	}
	else
		UE_LOG(LogTemp, Error, TEXT("3"));
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

void ASGLobbyPlayerController::Server_SetReady_Implementation(bool bNewReadyState)
{
	// [오직 서버에서만 실행] 내 컴퓨터에 매핑된 클라이언트의 PlayerState 수정
	ASGLobbyGameState* MyPlayerState = GetPlayerState<ASGLobbyGameState>();
	if (MyPlayerState)
	{
		MyPlayerState->bIsReady = bNewReadyState;
		UE_LOG(LogTemp, Log, TEXT("[Server] Player %s set ready to: %s"), *GetName(), bNewReadyState ? TEXT("TRUE") : TEXT("FALSE"));
	}
}

bool ASGLobbyPlayerController::Server_SetReady_Validate(bool bNewReadyState)
{
	return true;
}
