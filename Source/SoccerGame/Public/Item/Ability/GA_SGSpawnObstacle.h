// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Abilities/GameplayAbility.h"
#include "GA_SGSpawnObstacle.generated.h"

class ASGObstacleBase;
/**
 * 
 */
UCLASS()
class SOCCERGAME_API UGA_SGSpawnObstacle : public UGameplayAbility
{
	GENERATED_BODY()
	
public:
	UGA_SGSpawnObstacle();
	
protected:
	virtual void ActivateAbility(
		const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, 
		const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData) override;
	
private:
	UPROPERTY(EditDefaultsOnly, Category = "Item|Obstacle")
	TSubclassOf<ASGObstacleBase> ObstacleClass;
	
};
