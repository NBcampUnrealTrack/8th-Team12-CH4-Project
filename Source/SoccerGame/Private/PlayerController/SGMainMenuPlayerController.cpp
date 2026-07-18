// Fill out your copyright notice in the Description page of Project Settings.


#include "PlayerController/SGMainMenuPlayerController.h"
#include "Blueprint/UserWidget.h"

void ASGMainMenuPlayerController::BeginPlay()
{
	Super::BeginPlay();
	
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
	if (GameGuideWidgetClass)
	{
		GameGuideWidgetInstance =CreateWidget<UUserWidget>(this, GameGuideWidgetClass);
		if (IsValid(GameGuideWidgetInstance))
		{
			GameGuideWidgetInstance->AddToViewport(10);
			GameGuideWidgetInstance->SetVisibility(ESlateVisibility::Collapsed);
		}
	}
	bShowMouseCursor = true;
	FInputModeUIOnly InputModeData;
	if (IsValid(MainMenuWidgetInstance))
	{
		InputModeData.SetWidgetToFocus(MainMenuWidgetInstance->TakeWidget());
	}
	InputModeData.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);
	SetInputMode(InputModeData);
}

void ASGMainMenuPlayerController::OpenGameGuide()
{
	if (!IsLocalController())
	{
		return;
	}

	if (!GameGuideWidgetClass)
	{
		return;
	}
	if (!IsValid(GameGuideWidgetInstance))
	{
		GameGuideWidgetInstance =CreateWidget<UUserWidget>(this,GameGuideWidgetClass);
		if (!IsValid(GameGuideWidgetInstance))
		{
			return;
		}
	}
	if (!GameGuideWidgetInstance->IsInViewport())
	{
		GameGuideWidgetInstance->AddToViewport(10);
	}
	bShowMouseCursor = true;
	GameGuideWidgetInstance->SetVisibility(ESlateVisibility::Visible);

	FInputModeUIOnly InputModeData;
	InputModeData.SetWidgetToFocus(GameGuideWidgetInstance->TakeWidget());
	InputModeData.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);
	SetInputMode(InputModeData);
}

void ASGMainMenuPlayerController::CloseGameGuide()
{
	if (!IsLocalController())
	{
		return;
	}

	if (!IsValid(GameGuideWidgetInstance))
	{
		return;
	}

	GameGuideWidgetInstance->SetVisibility(ESlateVisibility::Collapsed);
	FInputModeUIOnly InputModeData;

	if (IsValid(MainMenuWidgetInstance))
	{
		InputModeData.SetWidgetToFocus(MainMenuWidgetInstance->TakeWidget());
	}

	InputModeData.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);

	SetInputMode(InputModeData);
	bShowMouseCursor = true;
}
