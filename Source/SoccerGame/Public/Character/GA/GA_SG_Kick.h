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
	// --- 애니메이션 설정 ---
	// 0.5초 미만일 때 재생할 가벼운 패스 몽타주
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Ability|Animation")
	TObjectPtr<UAnimMontage> PassMontage;

	// 0.5초 이상일 때 재생할 강력한 슛 몽타주
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Ability|Animation")
	TObjectPtr<UAnimMontage> ShootMontage;

	// 패스와 슛을 가르는 기준 시간 (디폴트 0.5초)
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Ability|Kick")
	float ActionSplitTime = 0.5f;
	
	// 최종 계산된 파워를 멤버 변수로 기억 (애니메이션 노티파이 시점에 쓰기 위함)
	float CachedFinalKickPower = 0.0f;
	
	// 충전이 시작된 시간
	float ChargeStartTime = 0.0f;
	
	// 발차기 최대 충전 시간
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ability|Kick")
	float MaxChargeTime = 2.0f;
	
	// 최대 충전시 발차기 위력 배율
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ability|Kick")
	float MaxPowerMultiplier = 2.0f;
	
	// 킥파워 차징 타이머 핸들
	FTimerHandle ChargeTimerHandle;
	
	// 차징 중이거나 킥 동작을 수행 중인지 여부
	bool bIsKickInProgress = false;

	// 축구공을 찾아서 힘을 가하는 함수
	UFUNCTION(BlueprintCallable, Category = "Ability|Kick")
	void FindAndPushBall();
	
	// BP_GE_SG_Kick 담는 변수
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Ability|Damage")
	TSubclassOf<UGameplayEffect> DamageEffectClass;
	
	
	UFUNCTION()
	void OnEnemyHitReceived(FGameplayEventData Payload);
	
	// 차징 후 마우스를 떼면 호출되는 함수
	virtual void InputReleased(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo) override;
	
	// WaitGameplayEvent 콜백 함수
	UFUNCTION()
	void OnGameplayEventReceived(FGameplayEventData Payload);
	
	// KickPower UI 업데이트 함수
	void UpdateKickPowerUI();
	
	
public:
	// 어빌리티가 실행될 수 있는 조건인지 서버/클라 양쪽에서 미리 검사하는 GAS 핵심 함수
	virtual bool CanActivateAbility(
		const FGameplayAbilitySpecHandle Handle, 
		const FGameplayAbilityActorInfo* ActorInfo, 
		const FGameplayTagContainer* SourceTags = nullptr, 
		const FGameplayTagContainer* TargetTags = nullptr, 
		FGameplayTagContainer* OptionalRelevantTags = nullptr
	) const override;
};
