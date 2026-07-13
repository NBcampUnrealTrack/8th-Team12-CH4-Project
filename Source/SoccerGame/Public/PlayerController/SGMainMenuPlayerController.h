// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "SGMainMenuPlayerController.generated.h"

class UUserWidget;
/**
 * 
 */
UCLASS()
class SOCCERGAME_API ASGMainMenuPlayerController : public APlayerController
{
	GENERATED_BODY()
	
protected:
	virtual void BeginPlay() override;
	
public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "UI")
	TSubclassOf<UUserWidget> MainMenuWidgetClass;
	
private:
	UPROPERTY()
	UUserWidget* MainMenuWidgetInstance;
};
