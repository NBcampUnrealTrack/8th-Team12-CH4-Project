// Fill out your copyright notice in the Description page of Project Settings.


#include "Item/Ability/GA_SGKickPowerUp.h"

#include "AbilitySystemComponent.h"

void UGA_SGKickPowerUp::ExecuteItemAbility(float TimeHeld)
{
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
