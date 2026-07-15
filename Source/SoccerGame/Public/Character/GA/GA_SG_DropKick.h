// GA_SG_DropKick.h

#pragma once

#include "CoreMinimal.h"
#include "Abilities/GameplayAbility.h"
#include "GA_SG_DropKick.generated.h"

UCLASS()
class SOCCERGAME_API UGA_SG_DropKick : public UGameplayAbility
{
	GENERATED_BODY()
	
public:
	UGA_SG_DropKick();
	
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
		bool bReplicateEndAbility, bool bWasCancelled
		) override;

protected:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Ability|Animation")
	TObjectPtr<UAnimMontage> DropKickMontage;
    
	// 축구공을 차는(드롭킥) 함수
	UFUNCTION(BlueprintCallable, Category = "Ability|DropKick")
	void PushBall(AActor* BallActor);
    
	// BP_GE_SG_DropKick 담는 변수
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Ability|Damage")
	TSubclassOf<UGameplayEffect> DamageEffectClass;
    
	// 타겟 하나를 지정해서 데미지를 주는 방식
	UFUNCTION()
	void ApplyDamageToTarget(AActor* HitEnemy, const FGameplayEventData& Payload);

	// 날아가는 동안(드롭킥) 매 프레임 호출될 콜백 함수
	UFUNCTION()
	void OnGameplayEventReceived(FGameplayEventData Payload);

private:
	// 중복 타격 방지 액터 리스트
	UPROPERTY()
	TArray<TObjectPtr<AActor>> AlreadyHitActors;
	
public:
	// ---------------------- DropKick 축구공 파워 배율, Z축 파워 변수 ---------------------- //
	// 드롭킥 파워 배율 (기본값: 2.0f)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "KickPowerSetting")
	float KickPowerMultiplier = 2.0f;

	// 공이 위로 뜨는 Z축 보정 비율 (기본값: 1.0f)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "KickPowerSetting")
	float UpwardForceRatio = 1.0f;
	
	// ---------------------- DropKick 래그돌 파워 배율, Z축 파워 변수 ---------------------- //
	// 드롭킥 래그돌 파워 배율 (기본값: 1.0f)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "KickPowerSetting")
	float RagdollKickPowerMultiplier = 1.0f;

	// 캐릭터가 위로 뜨는 Z축 보정 비율 (기본값: 0.5f)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "KickPowerSetting")
	float RagdollUpwardForceRatio = 0.5f;
};
