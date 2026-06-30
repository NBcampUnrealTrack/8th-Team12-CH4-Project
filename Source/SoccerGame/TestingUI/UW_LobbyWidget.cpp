// Fill out your copyright notice in the Description page of Project Settings.


#include "UW_LobbyWidget.h"

#include "Components/Button.h"

UUW_LobbyWidget::UUW_LobbyWidget(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)

{
}

void UUW_LobbyWidget::NativeConstruct()
{
	Super::NativeConstruct();
	PlayButton.Get()->OnClicked.AddDynamic(this, &UUW_LobbyWidget::OnPlayButtonClicked);
}

void UUW_LobbyWidget::OnPlayButtonClicked()
{
	UE_LOG(LogTemp,Warning,TEXT("[UUW_LobbyWidget::OnPlayButtonClicked]"));
}
