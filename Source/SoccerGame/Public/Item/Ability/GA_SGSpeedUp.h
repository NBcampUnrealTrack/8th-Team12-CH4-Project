// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Abilities/GameplayAbility.h"
#include "GA_SGSpeedUp.generated.h"

/**
 * 
 */
UCLASS()
class SOCCERGAME_API UGA_SGSpeedUp : public UGameplayAbility
{
	GENERATED_BODY()
	
public:
	UGA_SGSpeedUp();
	
protected:
	virtual void ActivateAbility(
		const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, 
		const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData) override;
	
	UFUNCTION()
	void OnInputReleased(float TimeHeld);
	
protected:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "GA|Buff")
	TSubclassOf<UGameplayEffect> SpeedUpEffect;
	
};
