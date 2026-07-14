// Fill out your copyright notice in the Description page of Project Settings.


#include "GameMode/SGMainMenuGameMode.h"

#include "PlayerController/SGMainMenuPlayerController.h"

ASGMainMenuGameMode::ASGMainMenuGameMode()
{
	PlayerControllerClass = ASGMainMenuPlayerController::StaticClass();
	
	DefaultPawnClass = nullptr;
	// 첫 레벨 이동 시의 안정성을 위해 SeamlessTravel은 비활성화
	bUseSeamlessTravel = false;
	
}