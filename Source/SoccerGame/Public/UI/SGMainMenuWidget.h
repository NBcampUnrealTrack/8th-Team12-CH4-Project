// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "SGMainMenuWidget.generated.h"

class UButton;
/**
 * 
 */
UCLASS()
class SOCCERGAME_API USGMainMenuWidget : public UUserWidget
{
	GENERATED_BODY()
	
protected:
	virtual void NativeConstruct() override;
	
private:
	UPROPERTY(meta = (BindWidget))
	UButton* GameStartButton;
	
	UFUNCTION()
	void OnGameStartButtonClicked();
};
