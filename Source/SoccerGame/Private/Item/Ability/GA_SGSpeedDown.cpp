// Fill out your copyright notice in the Description page of Project Settings.


#include "Item/Ability/GA_SGSpeedDown.h"

#include "AbilitySystemComponent.h"
#include "GameFramework/Controller.h"
#include "GameFramework/Pawn.h"
#include "GameFramework/PlayerController.h"
#include "GameplayTagContainer.h"
#include "PlayerState/SGMainPlayerState.h"

void UGA_SGSpeedDown::ExecuteItemAbility(float TimeHeld)
{
	if (SpeedDownEffect == nullptr){
		EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, true);
		return;
	}
	
	APawn* OwnerPawn = Cast<APawn>(CurrentActorInfo->AvatarActor.Get());
	AController* OwnerController = OwnerPawn ? OwnerPawn->GetController() : nullptr;
	ASGMainPlayerState* OwnerPlayerState =
		OwnerController ? OwnerController->GetPlayerState<ASGMainPlayerState>() : nullptr;
	
	const FGameplayTag WaitingTeamTag = FGameplayTag::RequestGameplayTag(FName("Team.Waiting"));
	if (OwnerPlayerState == nullptr || OwnerPlayerState->CurrentTeamTag == WaitingTeamTag){
		EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, true);
		return;
	}
	
	UWorld* World = GetWorld();
	if (World == nullptr){
		EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, true);
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
			TargetPlayerState->CurrentTeamTag == OwnerPlayerState->CurrentTeamTag ||
			TargetPlayerState->CurrentTeamTag == WaitingTeamTag) continue;
		
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
	
	EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, bWasCancelled);
}
