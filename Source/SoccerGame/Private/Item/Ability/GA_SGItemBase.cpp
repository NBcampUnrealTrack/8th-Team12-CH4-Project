// Fill out your copyright notice in the Description page of Project Settings.


#include "Item/Ability/GA_SGItemBase.h"

#include "Abilities/Tasks/AbilityTask_WaitInputRelease.h"

UGA_SGItemBase::UGA_SGItemBase()
{
	InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;
	NetExecutionPolicy = EGameplayAbilityNetExecutionPolicy::LocalPredicted;
}

void UGA_SGItemBase::ActivateAbility(const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo,
	const FGameplayEventData* TriggerEventData)
{
	if (ActorInfo == nullptr || !ActorInfo->AvatarActor.IsValid()){
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}
	
	UAbilityTask_WaitInputRelease* WaitInputReleaseTask = 
		UAbilityTask_WaitInputRelease::WaitInputRelease(this, true);
	
	WaitInputReleaseTask->OnRelease.AddDynamic(this, &UGA_SGItemBase::OnInputReleased);
	WaitInputReleaseTask->ReadyForActivation();
}

void UGA_SGItemBase::ExecuteItemAbility(float TimeHeld)
{
	// 실제 기능 구현
	EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, true);
}

void UGA_SGItemBase::HandleLocalInputReleased(float TimeHeld)
{
	// 필요한 경우 구현
}

void UGA_SGItemBase::OnInputReleased(float TimeHeld)
{
	if (CurrentActorInfo == nullptr || !CurrentActorInfo->AvatarActor.IsValid()){
		EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, true);
		return;
	}
	
	// Local 인 경우 호출
	if (CurrentActorInfo->IsLocallyControlled()){
		HandleLocalInputReleased(TimeHeld);	
	}
	
	// 서버가 아니라면 종료
	if (!CurrentActorInfo->AvatarActor->HasAuthority()){
		EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, false, false);
		return;
	}
	
	ExecuteItemAbility(TimeHeld);
}
