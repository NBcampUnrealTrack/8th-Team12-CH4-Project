// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Item/Ability/GA_SGItemBase.h"
#include "GA_SGSpeedDown.generated.h"

/**
 * 
 */
UCLASS()
class SOCCERGAME_API UGA_SGSpeedDown : public UGA_SGItemBase
{
	GENERATED_BODY()
	
protected:
	virtual void ExecuteItemAbility(float TimeHeld) override;
	
protected:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "GA|Debuff")
	TSubclassOf<UGameplayEffect> SpeedDownEffect;
};
