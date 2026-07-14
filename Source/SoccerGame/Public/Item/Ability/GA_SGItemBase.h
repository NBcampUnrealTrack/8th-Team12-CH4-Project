// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Abilities/GameplayAbility.h"
#include "GA_SGItemBase.generated.h"

/**
 * 
 */
UCLASS()
class SOCCERGAME_API UGA_SGItemBase : public UGameplayAbility
{
	GENERATED_BODY()
	
public:
	UGA_SGItemBase();
	
public:
	virtual void HandleRotateInput(float InputValue);

protected:
	virtual void ActivateAbility(
		const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, 
		const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData) override;
	
	virtual void HandleLocalInputReleased(float TimeHeld);
	virtual void ExecuteItemAbility(float TimeHeld);
	
private:
	UFUNCTION()
	void OnInputReleased(float TimeHeld);
};
