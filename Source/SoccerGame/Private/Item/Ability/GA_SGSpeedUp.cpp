// Fill out your copyright notice in the Description page of Project Settings.


#include "Item/Ability/GA_SGSpeedUp.h"

#include "AbilitySystemComponent.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"

// Character PR 이후 삭제
namespace
{
	struct FSGSpeedUpState
	{
		TWeakObjectPtr<UCharacterMovementComponent> MovementComponent;
		float OriginalMaxWalkSpeed = 0.f;
		FTimerHandle TimerHandle;
	};
	
	TMap<TWeakObjectPtr<AActor>, FSGSpeedUpState> SpeedUpStates;
	
	void RestoreSpeedUp(TWeakObjectPtr<AActor> AvatarActor)
	{
		FSGSpeedUpState* State = SpeedUpStates.Find(AvatarActor);
		if (State == nullptr) return;
		
		if (State->MovementComponent.IsValid()){
			State->MovementComponent->MaxWalkSpeed = State->OriginalMaxWalkSpeed;
		}
		
		SpeedUpStates.Remove(AvatarActor);
	}
	
	void ApplySpeedUpDirectly(AActor* AvatarActor, UCharacterMovementComponent* MovementComponent)
	{
		if (!IsValid(AvatarActor) || !IsValid(MovementComponent)) return;
	
		FSGSpeedUpState& State = SpeedUpStates.FindOrAdd(AvatarActor);
		if (!State.MovementComponent.IsValid()){
			State.MovementComponent = MovementComponent;
			State.OriginalMaxWalkSpeed = MovementComponent->MaxWalkSpeed;
		}
	
		MovementComponent->MaxWalkSpeed = State.OriginalMaxWalkSpeed * 1.5f;
	
		UWorld* World = AvatarActor->GetWorld();
		if (World == nullptr){
			RestoreSpeedUp(AvatarActor);
			return;
		}
		
		World->GetTimerManager().ClearTimer(State.TimerHandle);
		World->GetTimerManager().SetTimer(
			State.TimerHandle, 
			FTimerDelegate::CreateStatic(&RestoreSpeedUp, TWeakObjectPtr<AActor>(AvatarActor)),
			5.f, 
			false);
	}
}

UGA_SGSpeedUp::UGA_SGSpeedUp()
{
	InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;
	NetExecutionPolicy = EGameplayAbilityNetExecutionPolicy::ServerOnly;
}

void UGA_SGSpeedUp::ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo,
	const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData)
{
	if (ActorInfo == nullptr || !ActorInfo->AvatarActor.IsValid()){
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}
}

void UGA_SGSpeedUp::InputReleased(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo,
	const FGameplayAbilityActivationInfo ActivationInfo)
{
	if (ActorInfo == nullptr || !ActorInfo->AvatarActor.IsValid()){
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}
	
#pragma region GE 사용 로직 (Character PR 이후 활성화)
	/*
	UAbilitySystemComponent* AbilitySystemComponent = GetAbilitySystemComponentFromActorInfo();
	if (!IsValid(AbilitySystemComponent) || SpeedUpEffect == nullptr){
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}
	
	FGameplayEffectContextHandle EffectContext = AbilitySystemComponent->MakeEffectContext();
	EffectContext.AddSourceObject(this);
	
	FGameplayEffectSpecHandle EffectSpecHandle = AbilitySystemComponent->MakeOutgoingSpec(
		SpeedUpEffect, GetAbilityLevel(), EffectContext);
	
	if (!EffectSpecHandle.IsValid()){
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}
	
	AbilitySystemComponent->ApplyGameplayEffectSpecToSelf(*EffectSpecHandle.Data.Get());
	EndAbility(Handle, ActorInfo, ActivationInfo, true, false);
	*/
#pragma endregion
	
#pragma region GE 미사용 로직 (삭제 대상)
	ACharacter* Character = Cast<ACharacter>(ActorInfo->AvatarActor.Get());
	if (!IsValid(Character) || !IsValid(Character->GetCharacterMovement())){
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}
	
	ApplySpeedUpDirectly(ActorInfo->AvatarActor.Get(), Character->GetCharacterMovement());
	EndAbility(Handle, ActorInfo, ActivationInfo, false, false);
	
#pragma endregion
}
