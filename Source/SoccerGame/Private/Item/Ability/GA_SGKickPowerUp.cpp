// Fill out your copyright notice in the Description page of Project Settings.


#include "Item/Ability/GA_SGKickPowerUp.h"

#include "AbilitySystemComponent.h"
#include "Abilities/Tasks/AbilityTask_WaitInputRelease.h"

UGA_SGKickPowerUp::UGA_SGKickPowerUp()
{
	InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;
	NetExecutionPolicy = EGameplayAbilityNetExecutionPolicy::LocalPredicted;
}

void UGA_SGKickPowerUp::ActivateAbility(const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo,
	const FGameplayEventData* TriggerEventData)
{
	if (ActorInfo == nullptr || !ActorInfo->AvatarActor.IsValid()){
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}
	
	UAbilityTask_WaitInputRelease* WaitInputReleaseTask = 
		UAbilityTask_WaitInputRelease::WaitInputRelease(this, true);
	
	WaitInputReleaseTask->OnRelease.AddDynamic(this, &UGA_SGKickPowerUp::OnInputReleased);
	WaitInputReleaseTask->ReadyForActivation();
}

void UGA_SGKickPowerUp::OnInputReleased(float TimeHeld)
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
	if (!IsValid(AbilitySystemComponent) || KickPowerBuffEffect == nullptr){
		EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, true);
		return;
	}
	
	FGameplayEffectContextHandle  EffectContext = AbilitySystemComponent->MakeEffectContext();
	EffectContext.AddSourceObject(this);
	
	FGameplayEffectSpecHandle  EffectSpecHandle = AbilitySystemComponent->MakeOutgoingSpec(
		KickPowerBuffEffect, GetAbilityLevel(), EffectContext);
	
	if (!EffectSpecHandle.IsValid()){
		EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, true);
		return;
	}
	
	AbilitySystemComponent->ApplyGameplayEffectSpecToSelf(*EffectSpecHandle.Data.Get());

	EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, false);
}
