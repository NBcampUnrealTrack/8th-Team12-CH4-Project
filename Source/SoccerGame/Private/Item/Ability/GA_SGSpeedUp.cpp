// Fill out your copyright notice in the Description page of Project Settings.


#include "Item/Ability/GA_SGSpeedUp.h"

#include "AbilitySystemComponent.h"
#include "Abilities/Tasks/AbilityTask_WaitInputRelease.h"

UGA_SGSpeedUp::UGA_SGSpeedUp()
{
	InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;
	NetExecutionPolicy = EGameplayAbilityNetExecutionPolicy::LocalPredicted;
}

void UGA_SGSpeedUp::ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo,
	const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData)
{
	if (ActorInfo == nullptr || !ActorInfo->AvatarActor.IsValid()){
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}
	
	UAbilityTask_WaitInputRelease* WaitInputReleaseTask =
		UAbilityTask_WaitInputRelease::WaitInputRelease(this, true);
	
	WaitInputReleaseTask->OnRelease.AddDynamic(this, &UGA_SGSpeedUp::OnInputReleased);
	WaitInputReleaseTask->ReadyForActivation();
}

void UGA_SGSpeedUp::OnInputReleased(float TimeHeld)
{
	if (CurrentActorInfo == nullptr || !CurrentActorInfo->AvatarActor.IsValid()){
		EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, true);
		return;
	}
	
	if (!CurrentActorInfo->AvatarActor->HasAuthority()){
		EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, false, false);
		return;
	}
	
	UAbilitySystemComponent* AbilitySystemComponent = GetAbilitySystemComponentFromActorInfo();
	if (!IsValid(AbilitySystemComponent) || SpeedUpEffect == nullptr){
		EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, true);
		return;
	}
	
	FGameplayEffectContextHandle EffectContext = AbilitySystemComponent->MakeEffectContext();
	EffectContext.AddSourceObject(this);
	
	FGameplayEffectSpecHandle EffectSpecHandle = AbilitySystemComponent->MakeOutgoingSpec(
		SpeedUpEffect, GetAbilityLevel(), EffectContext);
	
	if (!EffectSpecHandle.IsValid()){
		EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, true);
		return;
	}
	
	AbilitySystemComponent->ApplyGameplayEffectSpecToSelf(*EffectSpecHandle.Data.Get());

	EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, false);
}
