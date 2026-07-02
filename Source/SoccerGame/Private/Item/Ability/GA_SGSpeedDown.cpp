// Fill out your copyright notice in the Description page of Project Settings.


#include "Item/Ability/GA_SGSpeedDown.h"

#include "AbilitySystemComponent.h"

UGA_SGSpeedDown::UGA_SGSpeedDown()
{
	InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;
	NetExecutionPolicy = EGameplayAbilityNetExecutionPolicy::ServerOnly;
}

void UGA_SGSpeedDown::ActivateAbility(const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo,
	const FGameplayEventData* TriggerEventData)
{
	if (ActorInfo == nullptr || !ActorInfo->AvatarActor.IsValid()){
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}
}

void UGA_SGSpeedDown::InputReleased(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo,
	const FGameplayAbilityActivationInfo ActivationInfo)
{
	if (ActorInfo == nullptr || !ActorInfo->AvatarActor.IsValid() || SpeedDownEffect == nullptr){
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}

#pragma region 네트워킹 PR 이후
	/*
	AActor* OwnerActor = ActorInfo->AvatarActor.Get();
	AController* OwnerController = Cast<AController>(OwnerActor->GetOwner());
	ASGMainPlayerState* OwnerPlayerState = 
		OwnerController ? OwnerController->GetPlayerState<ASGMainPlayerState>() : nullptr;
	
	if (OwnerPlayerState == nullptr || OwnerPlayerState->CurrentTeam == ESGPlayerTeam::Neutrality){
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}
	
	UWorld* World = GetWorld();
	if (World == nullptr){
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}
	
	bool bWasCancelled = true;
	
	// 적용할 player 탐색
	for (FConstPlayerControllerIterator It = World->GetPlayerControllerIterator(); It; ++It){
		APlayerController* TargetController = It->Get();
		if (!IsValid(TargetController)) continue;
		
		// 팀 식별
		ASGMainPlayerState* TargetPlayerState = 
			TargetController->GetPlayerState<ASGMainPlayerState>();
		if (TargetPlayerState == nullptr ||
			TargetPlayerState->CurrentTeam == OwnerPlayerState->CurrentTeam ||
			TargetPlayerState->CurrentTeam == ESGPlayerTeam::Neutrality) continue;
		
		APawn* TargetPawn = TargetController->GetPawn();
		if (!IsValid(TargetPawn)) continue;
		
		// Speed Multiplier 적용
		UAbilitySystemComponent* TargetAbilitySystemComponent = 
			TargetPawn->FindComponentByClass<UAbilitySystemComponent>();
		if (!IsValid(TargetAbilitySystemComponent)) continue;
		
		FGameplayEffectContextHandle EffectContext = TargetAbilitySystemComponent->MakeEffectContext();
		EffectContext.AddSourceObject(this);
		
		FGameplayEffectSpecHandle EffectSpecHandle = TargetAbilitySystemComponent->MakeOutgoingSpec(
			SpeedDownEffect, GetAbilityLevel(), EffectContext);
		if (!EffectSpecHandle.IsValid()) continue;
		
		TargetAbilitySystemComponent->ApplyGameplayEffectSpecToSelf(*EffectSpecHandle.Data.Get());
		bWasCancelled = false;
	}
	
	EndAbility(Handle, ActorInfo, ActivationInfo, true, bWasCancelled);
	*/
	EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
	
#pragma endregion
}
