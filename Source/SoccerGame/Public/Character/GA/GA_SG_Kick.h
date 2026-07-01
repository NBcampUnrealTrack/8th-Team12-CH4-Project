// GA_SG_Kick.h

#pragma once

#include "CoreMinimal.h"
#include "Abilities/GameplayAbility.h"
#include "GA_SG_Kick.generated.h"

UCLASS()
class SOCCERGAME_API UGA_SG_Kick : public UGameplayAbility
{
	GENERATED_BODY()
	
public:
	UGA_SG_Kick();
	
	virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData) override;
	
	virtual void EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled) override;
	
protected:
	// 발차기 애니메이션 몽타주(에디터에서 할당!)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation")
	TObjectPtr<UAnimMontage> KickMontage;
	
	// 충전이 시작된 시간
	float ChargeStartTime = 0.0f;
	
	// 발차기 최대 충전 시간
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ability|Kick")
	float MaxChargeTime = 2.0f;
	
	// 최대 충전시 발차기 위력 배율
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ability|Kick")
	float MaxPowerMultiplier = 2.0f;

	// 축구공을 찾아서 힘을 가하는 함수
	UFUNCTION(BlueprintCallable, Category = "Ability|Kick")
	void FindAndPushBall(float ChargeTime);
	
	// 차징 후 마우스를 떼면 호출되는 함수
	UFUNCTION()
	void OnInputReleased(float TimeHeld);
};
