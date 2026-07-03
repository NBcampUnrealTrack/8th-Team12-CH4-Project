// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "SGInGameWidget.generated.h"

class UTextBlock;
/**
 * 
 */
UCLASS()
class SOCCERGAME_API USGInGameWidget : public UUserWidget
{
	GENERATED_BODY()
	
public:
	virtual void NativeConstruct() override;
	
	// GameState의 Broadcast로 실행될 함수
	UFUNCTION()
	void UpdateTimerText(int32 CurrentTime);
	
protected:
	// 타이머 텍스트
	UPROPERTY(meta = (BindWidget))
	UTextBlock* Text_Timer;
	
};
