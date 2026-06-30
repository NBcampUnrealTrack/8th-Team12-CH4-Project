// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "UW_LobbyWidget.generated.h"
class UButton;
class UEditableText;
/**
 * 
 */
UCLASS()
class SOCCERGAME_API UUW_LobbyWidget : public UUserWidget
{
	GENERATED_BODY()
public:
	UUW_LobbyWidget(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());
	
protected:
	virtual void NativeConstruct() override;

	UFUNCTION()
	void OnPlayButtonClicked();
	
private:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = USTitleWidget, Meta = (AllowPrivateAccess, BindWidget)) 
	TObjectPtr<UButton> PlayButton;
	
	
	
};
