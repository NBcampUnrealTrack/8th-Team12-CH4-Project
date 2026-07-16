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
	
	// 어빌리티가 실행될 수 있는 조건인지 서버/클라 양쪽에서 미리 검사하는 GAS 핵심 함수
	virtual bool CanActivateAbility(
		const FGameplayAbilitySpecHandle Handle, 
		const FGameplayAbilityActorInfo* ActorInfo, 
		const FGameplayTagContainer* SourceTags = nullptr, 
		const FGameplayTagContainer* TargetTags = nullptr, 
		FGameplayTagContainer* OptionalRelevantTags = nullptr
	) const override;
	
	virtual void ActivateAbility(
		const FGameplayAbilitySpecHandle Handle, 
		const FGameplayAbilityActorInfo* ActorInfo, 
		const FGameplayAbilityActivationInfo ActivationInfo, 
		const FGameplayEventData* TriggerEventData
		) override;
	
	virtual void EndAbility(
		const FGameplayAbilitySpecHandle Handle, 
		const FGameplayAbilityActorInfo* ActorInfo, 
		const FGameplayAbilityActivationInfo ActivationInfo, 
		bool bReplicateEndAbility, 
		bool bWasCancelled
		) override;
	
protected:
	// kick 몽타주
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Ability|Animation")
	TObjectPtr<UAnimMontage> KickMontage;
	
	// 축구공을 찾아서 힘을 가하는 함수 (Notify 시점 호출)
	UFUNCTION(BlueprintCallable, Category = "Ability|Kick")
	void FindAndPushBall();
	
	// BP_GE_SG_Kick 담는 변수
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Ability|Damage")
	TSubclassOf<UGameplayEffect> DamageEffectClass;
	
	// 데미지 적용 함수
	UFUNCTION()
	void OnEnemyHitReceived(FGameplayEventData Payload);
	
	// WaitGameplayEvent 콜백 함수
	UFUNCTION()
	void OnGameplayEventReceived(FGameplayEventData Payload);
	
private:
	// 연타/중복 실행 방지 플래그
	bool bIsKickInProgress = false;
	
	// 중복 타격 방지 액터 리스트
	UPROPERTY()
	TArray<TObjectPtr<AActor>> AlreadyHitActors;

public:
	// ------------------------ Kick 파워 배율, Z축 파워 변수 ------------------------ //
	// 킥 파워 배율 (기본값: 1.0f)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "KickPowerSetting")
	float KickPowerMultiplier = 1.0f;

	// 공이 위로 뜨는 Z축 보정 비율 (기본값: 0.7f)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "KickPowerSetting")
	float UpwardForceRatio = 0.7f;
};
