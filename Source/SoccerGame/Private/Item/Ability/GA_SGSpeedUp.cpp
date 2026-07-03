// Fill out your copyright notice in the Description page of Project Settings.


#include "Item/Ability/GA_SGSpeedUp.h"

#include "AbilitySystemComponent.h"

void UGA_SGSpeedUp::ExecuteItemAbility(float TimeHeld)
{
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
