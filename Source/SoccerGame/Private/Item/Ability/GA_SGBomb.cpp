// Fill out your copyright notice in the Description page of Project Settings.


#include "Item/Ability/GA_SGBomb.h"

#include "Engine/OverlapResult.h"
#include "GameFramework/Character.h"
#include "Item/Preview/SGProjectileBase.h"

UGA_SGBomb::UGA_SGBomb() :
	TargetDistance(600.f),
	ThrowSpeed(800.f),
	ThrowForwardOffset(100.f),
	ThrowHeightOffset(80.f),
	AreaImpulseRadius(350.f),
	AreaImpulseStrength(1000.f),
	AreaImpulseUpRatio(0.3f)
{
}

void UGA_SGBomb::ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo,
                                 const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData)
{
	Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);
	
	if (ActorInfo == nullptr || !ActorInfo->AvatarActor.IsValid() || !ActorInfo->IsLocallyControlled()) return;
	
	AActor* PlayerActor = ActorInfo->AvatarActor.Get();
	UWorld* World = GetWorld();
	if (World == nullptr || BombProjectileClass == nullptr) return;
	
	// Spawn 설정
	FActorSpawnParameters SpawnParams;
	SpawnParams.Owner = PlayerActor;
	SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
	
	// Preview 생성
	PreviewProjectileActor = World->SpawnActor<ASGProjectileBase>(
		BombProjectileClass,
		PlayerActor->GetActorLocation(),
		PlayerActor->GetActorRotation(),
		SpawnParams);
	if (!IsValid(PreviewProjectileActor)) return;
	PreviewProjectileActor->InitializePreview(
		PlayerActor,
		TargetDistance,
		ThrowSpeed,
		ThrowForwardOffset,
		ThrowHeightOffset);
}

void UGA_SGBomb::HandleLocalInputReleased(float TimeHeld)
{
	//Preview 정리
	if (IsValid(PreviewProjectileActor)){
		PreviewProjectileActor->Destroy();
	}
	PreviewProjectileActor = nullptr;
}

void UGA_SGBomb::ExecuteItemAbility(float TimeHeld)
{
	if (CurrentActorInfo == nullptr || !CurrentActorInfo->AvatarActor.IsValid() || BombProjectileClass == nullptr){
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
		BombProjectileClass,
		PlayerActor->GetActorLocation(),
		PlayerActor->GetActorRotation(),
		SpawnParams);
	
	if (!IsValid(Projectile) || !Projectile->LaunchByTrajectory(
		PlayerActor, TargetDistance, ThrowSpeed, ThrowForwardOffset, ThrowHeightOffset)){
		EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, true);
		return;
	}
	
	Projectile->OnProjectileFinished.AddDynamic(this, &UGA_SGBomb::HandleProjectileFinished);
}

void UGA_SGBomb::HandleProjectileFinished(ASGProjectileBase* Projectile)
{
	if (!IsActive() || Projectile == nullptr) return;
	
	ApplyAreaImpulse(Projectile->GetActorLocation());
	EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, false);
}

void UGA_SGBomb::ApplyAreaImpulse(const FVector& Origin)
{
	UWorld* World = GetWorld();
	if (World == nullptr) return;

	// Collision 감지 설정
	TArray<FOverlapResult> Overlaps;
	FCollisionObjectQueryParams ObjectQueryParams;
	ObjectQueryParams.AddObjectTypesToQuery(ECC_Pawn);
	const FCollisionShape CollisionShape = FCollisionShape::MakeSphere(AreaImpulseRadius);
	
	// Collision 감지
	if (!World->OverlapMultiByObjectType(
		Overlaps, Origin, FQuat::Identity, ObjectQueryParams, CollisionShape)) return;
	
	AActor* PlayerActor = CurrentActorInfo != nullptr && CurrentActorInfo->AvatarActor.IsValid() 
		? CurrentActorInfo->AvatarActor.Get() : nullptr;
	
	TSet<TObjectPtr<ACharacter>> HitCharacters;
	
	for (const FOverlapResult& Overlap : Overlaps){
		// 대상인 Character 인지 확인
		ACharacter* TargetCharacter = Cast<ACharacter>(Overlap.GetActor());
		if (!IsValid(TargetCharacter) || TargetCharacter == PlayerActor || HitCharacters.Contains(TargetCharacter)) continue;
		
		HitCharacters.Add(TargetCharacter);
		
		// Impulse 방향 설정
		FVector Direction = TargetCharacter->GetActorLocation() - Origin;
		if (Direction.IsNearlyZero()) Direction = FVector::UpVector;
		
		Direction = Direction.GetSafeNormal();
		Direction.Z = AreaImpulseUpRatio;
		Direction.Normalize();
		
		// Impulse 적용
		TargetCharacter->LaunchCharacter(Direction * AreaImpulseStrength, true, true);
	}
}
