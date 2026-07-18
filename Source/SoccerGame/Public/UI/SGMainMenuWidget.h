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
public:
	UFUNCTION(BlueprintCallable)
	void CloseGameGuide();
	
protected:
	virtual void NativeConstruct() override;
	
private:
	UPROPERTY(meta = (BindWidget))
	UButton* GameStartButton;
	
	UPROPERTY(meta = (BindWidget))
	UButton* GameGuideButton;
	
	UFUNCTION()
	void OnGameGuidButtonClicked();
	
	UFUNCTION()
	void OnGameStartButtonClicked();
};
