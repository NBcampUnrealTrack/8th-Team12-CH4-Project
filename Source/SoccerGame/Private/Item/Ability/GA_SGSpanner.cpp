// Fill out your copyright notice in the Description page of Project Settings.


#include "Item/Ability/GA_SGSpanner.h"

#include "Item/Preview/SGProjectileBase.h"

UGA_SGSpanner::UGA_SGSpanner() : 
	TargetDistance(800.f), 
	ThrowSpeed(1200.f), 
	ThrowForwardOffset(100.f),
	ThrowHeightOffset(80.f)
{
}

void UGA_SGSpanner::ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo,
	const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData)
{
	Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);
	
	// 로컬에서 조종 중인 플레이어만 Preview 생성
	if (ActorInfo == nullptr || !ActorInfo->AvatarActor.IsValid() || !ActorInfo->IsLocallyControlled()) return;
	
	AActor* PlayerActor = ActorInfo->AvatarActor.Get();
	
	UWorld* World = GetWorld();
	if (World == nullptr || SpannerProjectileClass == nullptr) return;
	
	// Preview Spawn 설정
	FActorSpawnParameters SpawnParams;
	SpawnParams.Owner = PlayerActor;
	SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
	
	// Preview 생성
	PreviewProjectileActor = World->SpawnActor<ASGProjectileBase>(
		SpannerProjectileClass, PlayerActor->GetActorLocation(), PlayerActor->GetActorRotation(), SpawnParams);
	if (IsValid(PreviewProjectileActor)){
		PreviewProjectileActor->InitializePreview(
			PlayerActor,
			TargetDistance,
			ThrowSpeed,
			ThrowForwardOffset,
			ThrowHeightOffset);
	}
}

void UGA_SGSpanner::HandleLocalInputReleased(float TimeHeld)
{
	// Preview 정리
	if (IsValid(PreviewProjectileActor)){
		PreviewProjectileActor->Destroy();
	}
	PreviewProjectileActor = nullptr;
}

void UGA_SGSpanner::ExecuteItemAbility(float TimeHeld)
{
	if (CurrentActorInfo == nullptr || !CurrentActorInfo->AvatarActor.IsValid() || SpannerProjectileClass == nullptr){
		EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, true);
		return;
	}
	
	AActor* PlayerActor = CurrentActorInfo->AvatarActor.Get();
	UWorld* World = GetWorld();
	
	if (World == nullptr){
		EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, true);
		return;
	}
	
	// Spawn 설정
	FActorSpawnParameters SpawnParams;
	SpawnParams.Owner = PlayerActor;
	SpawnParams.Instigator = Cast<APawn>(PlayerActor);
	SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
	
	// 발사체 생성
	ASGProjectileBase* Projectile = World->SpawnActor<ASGProjectileBase>(
		SpannerProjectileClass, PlayerActor->GetActorLocation(), PlayerActor->GetActorRotation(), SpawnParams);
	
	const bool bFailed = !IsValid(Projectile) || !Projectile->LaunchByTrajectory(
			PlayerActor,
			TargetDistance,
			ThrowSpeed,
			ThrowForwardOffset,
			ThrowHeightOffset);

	EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, !bFailed, !bFailed);
}