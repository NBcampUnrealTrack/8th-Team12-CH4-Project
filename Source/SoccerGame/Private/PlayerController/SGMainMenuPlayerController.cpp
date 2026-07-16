// Fill out your copyright notice in the Description page of Project Settings.


#include "PlayerController/SGMainMenuPlayerController.h"
#include "Blueprint/UserWidget.h"

void ASGMainMenuPlayerController::BeginPlay()
{
	Super::BeginPlay();
	
	UE_LOG(
		LogTemp,
		Error,
		TEXT(
			"[MainMenuPC BeginPlay] PC=%s World=%s Map=%s NetMode=%d"
		),
		*GetNameSafe(this),
		*GetNameSafe(GetWorld()),
		*GetWorld()->GetMapName(),
		static_cast<int32>(GetNetMode())
	);	
	if (IsLocalController() && MainMenuWidgetClass != nullptr)
	{
		MainMenuWidgetInstance = CreateWidget<UUserWidget>(this, MainMenuWidgetClass);
		
		if (MainMenuWidgetInstance != nullptr)
		{
			MainMenuWidgetInstance->AddToViewport();
			
			bShowMouseCursor = true;
			
			FInputModeUIOnly InputModeData;
			InputModeData.SetWidgetToFocus(MainMenuWidgetInstance->TakeWidget());
			//InputModeData.SetWidgetToFocus(MainMenuWidgetInstance->GetCachedWidget());
			InputModeData.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);
			SetInputMode(InputModeData);
		}
	}
}
