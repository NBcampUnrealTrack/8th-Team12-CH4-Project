// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "SGGameResultWidget.generated.h"

class UTextBlock;
class UButton;
/**
 * 
 */
UCLASS()
class SOCCERGAME_API USGGameResultWidget : public UUserWidget
{
	GENERATED_BODY()
	
protected:
	virtual void NativeConstruct() override;
	
#pragma region UMG Binding
	
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> Text_WinMessage;
	
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> Text_BlueTeamScore;
	
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> Text_RedTeamScore;
	
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UButton> Button_ExitGame; 
	
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UButton> Button_Rematch; 

#pragma endregion
	
protected:
	UFUNCTION()
	void OnExitGameClicked();
	
	UFUNCTION()
	void OnRematchClicked();
};
