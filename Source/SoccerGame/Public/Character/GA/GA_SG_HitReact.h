// GA_SG_HitReact.h

#pragma once

#include "CoreMinimal.h"
#include "Abilities/GameplayAbility.h"
#include "GA_SG_HitReact.generated.h"

class UAnimMontage;

UCLASS()
class SOCCERGAME_API UGA_SG_HitReact : public UGameplayAbility
{
	GENERATED_BODY()

public:
	UGA_SG_HitReact();

	virtual void ActivateAbility(
		const FGameplayAbilitySpecHandle Handle, 
		const FGameplayAbilityActorInfo* ActorInfo, 
		const FGameplayAbilityActivationInfo ActivationInfo, 
		const FGameplayEventData* TriggerEventData
		) override;

protected:
	// 피격 몽타주
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "HitReact")
	TObjectPtr<UAnimMontage> HitReactMontage;

	// 몽타주 재생 완료 및 중단 시 호출될 콜백
	UFUNCTION()
	void OnHitReactEnded();
	
	virtual void EndAbility(
		const FGameplayAbilitySpecHandle Handle, 
		const FGameplayAbilityActorInfo* ActorInfo, 
		const FGameplayAbilityActivationInfo ActivationInfo, 
		bool bReplicateEndAbility, bool bWasCancelled
		) override;
};
