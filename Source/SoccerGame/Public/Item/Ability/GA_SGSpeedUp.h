// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GA_SGItemBase.h"
#include "Abilities/GameplayAbility.h"
#include "GA_SGSpeedUp.generated.h"

/**
 * 
 */
UCLASS()
class SOCCERGAME_API UGA_SGSpeedUp : public UGA_SGItemBase
{
	GENERATED_BODY()
	
protected:
	virtual void ExecuteItemAbility(float TimeHeld);
	
protected:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "GA|Buff")
	TSubclassOf<UGameplayEffect> SpeedUpEffect;
	
};
