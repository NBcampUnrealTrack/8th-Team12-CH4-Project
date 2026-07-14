// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Item/Ability/GA_SGItemBase.h"
#include "GA_SGBomb.generated.h"

class ASGProjectileBase;
/**
 * 
 */
UCLASS()
class SOCCERGAME_API UGA_SGBomb : public UGA_SGItemBase
{
	GENERATED_BODY()
	
public:
	UGA_SGBomb();
	
protected:
	virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, 
		const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData) override;
	
	virtual void HandleLocalInputReleased(float TimeHeld) override;
	virtual void ExecuteItemAbility(float TimeHeld) override;
	
private:
	UFUNCTION()
	void HandleProjectileFinished(ASGProjectileBase* Projectile);
	void ApplyAreaImpulse(const FVector& Origin);
	
private:
	UPROPERTY(EditDefaultsOnly, Category = "Item")
	TSubclassOf<ASGProjectileBase> BombProjectileClass;
	
	UPROPERTY()
	TObjectPtr<ASGProjectileBase> PreviewProjectileActor;
	
	UPROPERTY(EditAnywhere, Category = "Item|Projectile")
	float TargetDistance;
	
	UPROPERTY(EditAnywhere, Category = "Item|Projectile")
	float ThrowSpeed;
	
	UPROPERTY(EditAnywhere, Category = "Item|Projectile")
	float ThrowForwardOffset;
	
	UPROPERTY(EditAnywhere, Category = "Item|Projectile")
	float ThrowHeightOffset;
	
	UPROPERTY(EditAnywhere, Category = "Item|Impulse")
	float AreaImpulseRadius;
	
	UPROPERTY(EditAnywhere, Category = "Item|Impulse")
	float AreaImpulseStrength;
	
	UPROPERTY(EditAnywhere, Category = "Item|Impulse")
	float AreaImpulseUpRatio;
};
