// GA_SG_HitReact.h

#include "Character/GA/GA_SG_HitReact.h"
#include "AbilitySystemComponent.h"
#include "Abilities/Tasks/AbilityTask_PlayMontageAndWait.h"
#include "Character/GAS/GAS_SG_CharacterAttributeSet.h"

UGA_SG_HitReact::UGA_SG_HitReact()
{
	InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;
	
	FAbilityTriggerData TriggerData;
	TriggerData.TriggerTag = FGameplayTag::RequestGameplayTag(FName("Character.HitReact.Kick"));
	TriggerData.TriggerSource = EGameplayAbilityTriggerSource::GameplayEvent;
	AbilityTriggers.Add(TriggerData);
	
	const FGameplayTag ImmunityTag = FGameplayTag::RequestGameplayTag(FName("State.Immunity"));
	AbilityTags.AddTag(ImmunityTag);
}

void UGA_SG_HitReact::ActivateAbility(
	const FGameplayAbilitySpecHandle Handle, 
	const FGameplayAbilityActorInfo* ActorInfo, 
	const FGameplayAbilityActivationInfo ActivationInfo, 
	const FGameplayEventData* TriggerEventData)
{
	Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);

	UAbilitySystemComponent* ASC = GetAbilitySystemComponentFromActorInfo();
	if (!ASC)
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}

	// 무적(Immunity) 태그 부여하여 다운 동안 추가 피격 방지
	const FGameplayTag ImmunityTag = FGameplayTag::RequestGameplayTag(FName("State.Immunity"));
	ASC->AddLooseGameplayTag(ImmunityTag);
	
	if (HitReactMontage)
	{
		UAbilityTask_PlayMontageAndWait* MontageTask = UAbilityTask_PlayMontageAndWait::CreatePlayMontageAndWaitProxy(
			this, NAME_None, HitReactMontage, 1.0f, NAME_None, true
		);

		if (MontageTask)
		{
			MontageTask->OnCompleted.AddDynamic(this, &UGA_SG_HitReact::OnHitReactEnded);
			MontageTask->OnInterrupted.AddDynamic(this, &UGA_SG_HitReact::OnHitReactEnded);
			MontageTask->OnCancelled.AddDynamic(this, &UGA_SG_HitReact::OnHitReactEnded);
			MontageTask->ReadyForActivation();
			return;
		}
	}
	
	OnHitReactEnded();
}

void UGA_SG_HitReact::OnHitReactEnded()
{
	EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, false);
}

void UGA_SG_HitReact::EndAbility(
	const FGameplayAbilitySpecHandle Handle, 
	const FGameplayAbilityActorInfo* ActorInfo,
	const FGameplayAbilityActivationInfo ActivationInfo,
	bool bReplicateEndAbility, 
	bool bWasCancelled)
{
	UAbilitySystemComponent* ASC = GetAbilitySystemComponentFromActorInfo();
	if (ASC)
	{
		// 무적 태그 해제
		const FGameplayTag ImmunityTag = FGameplayTag::RequestGameplayTag(FName("State.Immunity"));
		ASC->RemoveLooseGameplayTag(ImmunityTag);

		// HP 회복
		if (HasAuthority(&CurrentActivationInfo))
		{
			if (UGAS_SG_CharacterAttributeSet* AttributeSet = const_cast<UGAS_SG_CharacterAttributeSet*>(ASC->GetSet<UGAS_SG_CharacterAttributeSet>()))
			{
				const float MaxHp = AttributeSet->GetMaxHp();
				AttributeSet->SetHp(MaxHp);
			}
		}
	}
	
	Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);
}

