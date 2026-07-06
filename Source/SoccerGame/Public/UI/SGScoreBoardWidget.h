// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "SGScoreBoardWidget.generated.h"

class UTextBlock;
/**
 * 
 */
UCLASS()
class SOCCERGAME_API USGScoreBoardWidget : public UUserWidget
{
	GENERATED_BODY()
	
public:
	void UpdateScores(int32 BlueScore, int32 RedScore);
	
protected:
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> Text_BlueScore;
	
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> Text_RedScore;
};
