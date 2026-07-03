// Fill out your copyright notice in the Description page of Project Settings.


#include "PlayerController/SGMainPlayerController.h"

void ASGMainPlayerController::BeginPlay()
{
	Super::BeginPlay();

	if (IsLocalController())
	{
		FInputModeGameOnly InputMode;
		SetInputMode(InputMode);
		bShowMouseCursor = false;
	}
}
