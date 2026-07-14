// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/SGMainMenuWidget.h"
#include "Components/Button.h"
#include "Multiplay/SGMultiplayGameInstance.h"
#include "Kismet/GameplayStatics.h"


void USGMainMenuWidget::NativeConstruct()
{
	Super::NativeConstruct();
	

	if (GameStartButton)
	{
		// 델리게이트 바인딩
		GameStartButton->OnClicked.AddDynamic(this, &USGMainMenuWidget::OnGameStartButtonClicked);
	}
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
}
