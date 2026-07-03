// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Abilities/GameplayAbility.h"
#include "GA_SGSpeedDown.generated.h"

/**
 * 
 */
UCLASS()
class SOCCERGAME_API UGA_SGSpeedDown : public UGameplayAbility
{
	GENERATED_BODY()
	
public:
	UGA_SGSpeedDown();
	
protected:
	virtual void ActivateAbility(
		const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, 
		const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData) override;
	
	UFUNCTION()
	void OnInputReleased(float TimeHeld);
	
protected:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "GA|Debuff")
	TSubclassOf<UGameplayEffect> SpeedDownEffect;
};
