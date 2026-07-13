// Fill out your copyright notice in the Description page of Project Settings.


#include "GameMode/SGMainMenuGameMode.h"

#include "PlayerController/SGMainMenuPlayerController.h"

ASGMainMenuGameMode::ASGMainMenuGameMode()
{
	PlayerControllerClass = ASGMainMenuPlayerController::StaticClass();
	
	DefaultPawnClass = nullptr;
	
}
