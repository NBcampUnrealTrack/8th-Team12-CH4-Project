// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Item/Ability/GA_SGItemBase.h"
#include "GA_SGSpanner.generated.h"

class ASGProjectileBase;
/**
 * 
 */
UCLASS()
class SOCCERGAME_API UGA_SGSpanner : public UGA_SGItemBase
{
	GENERATED_BODY()
	
public:
	UGA_SGSpanner();

protected:
	virtual void ActivateAbility(
		const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, 
		const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData) override;
	
	virtual void HandleLocalInputReleased(float TimeHeld) override;
	
	virtual void ExecuteItemAbility(float TimeHeld) override;
	
private:
	// 발사체
	UPROPERTY(EditDefaultsOnly, Category = "Item|Spanner")
	TSubclassOf<ASGProjectileBase> SpannerProjectileClass;
	
	UPROPERTY()
	TObjectPtr<ASGProjectileBase> PreviewProjectileActor;
	
	// 발사체 설정
	UPROPERTY(EditAnywhere, Category = "Item|Spanner")
	float TargetDistance;
	
	UPROPERTY(EditAnywhere, Category = "Item|Spanner")
	float ThrowSpeed;
	
	// 발사체 Spawn 위치 설정
	UPROPERTY(EditAnywhere, Category = "Item|Spanner")
	float ThrowForwardOffset;
	
	UPROPERTY(EditAnywhere, Category = "Item|Spanner")
	float ThrowHeightOffset;
};
