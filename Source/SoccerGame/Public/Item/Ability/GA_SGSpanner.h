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
	// 델리게이트 수신 함수
	UFUNCTION()
	void HandleProjectileHit(ASGProjectileBase* Projectile, AActor* TargetActor);
	
	UFUNCTION()
	void HandleProjectileFinished(ASGProjectileBase* Projectile);
	
	// Impulse 적용
	void ApplyHitImpulse(ASGProjectileBase* Projectile, AActor* TargetActor);
	
	// Damage 적용
	bool IsOtherTeam(AActor* TargetActor) const;
	void ApplyDamageEffect(AActor* TargetActor);
	
private:
	// 발사체
	UPROPERTY(EditDefaultsOnly, Category = "Item")
	TSubclassOf<ASGProjectileBase> SpannerProjectileClass;
	
	UPROPERTY()
	TObjectPtr<ASGProjectileBase> PreviewProjectileActor;
	
	//GE
	UPROPERTY(EditDefaultsOnly, Category = "Item|GameplayEffect")
	TSubclassOf<UGameplayEffect> DamageEffectClass;
	
	// 발사체 설정
	UPROPERTY(EditAnywhere, Category = "Item")
	float TargetDistance;
	
	UPROPERTY(EditAnywhere, Category = "Item")
	float ThrowSpeed;
	
	// 발사체 Spawn 위치 설정
	UPROPERTY(EditAnywhere, Category = "Item")
	float ThrowForwardOffset;
	
	UPROPERTY(EditAnywhere, Category = "Item")
	float ThrowHeightOffset;
	
	// Impulse
	UPROPERTY(EditAnywhere, Category = "Item|Impulse")
	float HitImpulseStrength;
	
	UPROPERTY(EditAnywhere, Category = "Item|Impulse")
	float HitImpulseUpRatio;
};
