// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/SGMainMenuWidget.h"
#include "Components/Button.h"
#include "Multiplay/SGMultiplayGameInstance.h"
#include "PlayerController/SGMainMenuPlayerController.h"


void USGMainMenuWidget::CloseGameGuide()
{
	ASGMainMenuPlayerController* MainMenuPC =GetOwningPlayer<ASGMainMenuPlayerController>();
	if (IsValid(MainMenuPC))
	{
		MainMenuPC->CloseGameGuide();
	}
}

void USGMainMenuWidget::NativeConstruct()
{
	Super::NativeConstruct();
	

	if (GameStartButton)
	{
		// 델리게이트 바인딩
		GameStartButton->OnClicked.RemoveDynamic(this,&USGMainMenuWidget::OnGameStartButtonClicked);
		GameStartButton->OnClicked.AddDynamic(this, &USGMainMenuWidget::OnGameStartButtonClicked);
	}
	
	if (GameGuideButton)
	{
		GameGuideButton->OnClicked.RemoveDynamic(this, &USGMainMenuWidget::OnGameGuidButtonClicked);
		GameGuideButton->OnClicked.AddDynamic(this, &USGMainMenuWidget::OnGameGuidButtonClicked);
	}
}

void USGMainMenuWidget::OnGameGuidButtonClicked()
{
	ASGMainMenuPlayerController* MainMenuPC =GetOwningPlayer<ASGMainMenuPlayerController>();

	if (!IsValid(MainMenuPC))
	{
		return;
	}

	MainMenuPC->OpenGameGuide();
}

void USGMainMenuWidget::OnGameStartButtonClicked()
{
	UGameInstance* GameInstance = GetGameInstance();
	USGMultiplayGameInstance* SGGameInstance = Cast<USGMultiplayGameInstance>(GameInstance);
	
	if (SGGameInstance)
	{
		//SGGameInstance->CreateServer();
		SGGameInstance->FindServers();
	}
	else
	{
		UE_LOG(LogTemp,Warning,TEXT(" SGGameInstance : None "));
	}
}
