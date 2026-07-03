// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Item/Ability/GA_SGItemBase.h"
#include "GA_SGSpawnObstacle.generated.h"

class ASGObstacleBase;
/**
 * 
 */
UCLASS()
class SOCCERGAME_API UGA_SGSpawnObstacle : public UGA_SGItemBase
{
	GENERATED_BODY()
	
public:
	UGA_SGSpawnObstacle();
	
protected:
	virtual void ActivateAbility(
		const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, 
		const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData) override;
	
	virtual void HandleLocalInputReleased(float TimeHeld) override;
	virtual void ExecuteItemAbility(float TimeHeld) override;
	
private:
	void SpawnPreviewActor(const FGameplayAbilityActorInfo* ActorInfo);
	void DestroyPreviewActor();
	
protected:
	UPROPERTY(EditAnywhere, Category = "Item|Obstacle")
	float PreviewOpacity;
	
private:
	UPROPERTY(EditDefaultsOnly, Category = "Item|Obstacle")
	TSubclassOf<ASGObstacleBase> ObstacleClass;
	
	UPROPERTY(EditAnywhere, Category = "Item|Obstacle")
	float SpawnForwardDistance;
	
	UPROPERTY()
	TObjectPtr<ASGObstacleBase> PreviewActor;
};
