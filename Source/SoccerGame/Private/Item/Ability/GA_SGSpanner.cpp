// Fill out your copyright notice in the Description page of Project Settings.


#include "Item/Ability/GA_SGSpanner.h"

#include "AbilitySystemComponent.h"
#include "AbilitySystemGlobals.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Item/Preview/SGProjectileBase.h"
#include "PlayerState/SGMainPlayerState.h"

UGA_SGSpanner::UGA_SGSpanner() : 
	TargetDistance(800.f), 
	ThrowSpeed(1200.f), 
	ThrowForwardOffset(100.f),
	ThrowHeightOffset(80.f),
	HitImpulseStrength(800.f),
	HitImpulseUpRatio(0.3f)
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
	
	if (!IsValid(Projectile) || !Projectile->LaunchByTrajectory(
		PlayerActor, TargetDistance, ThrowSpeed, ThrowForwardOffset, ThrowHeightOffset)){
		EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, true);
		return;
	}

	Projectile->OnProjectileHitTarget.AddDynamic(this, &UGA_SGSpanner::HandleProjectileHit);
	Projectile->OnProjectileFinished.AddDynamic(this, &UGA_SGSpanner::HandleProjectileFinished);
}

void UGA_SGSpanner::HandleProjectileHit(ASGProjectileBase* Projectile, AActor* TargetActor)
{
	if (!IsValid(TargetActor)) return;
	
	// Impulse 적용
	ApplyHitImpulse(Projectile, TargetActor);
	
	// 다른 팀인 경우 데미지 적용
	if (!IsOtherTeam(TargetActor)) return;
	ApplyDamageEffect(TargetActor);
}

void UGA_SGSpanner::HandleProjectileFinished(ASGProjectileBase* Projectile)
{
	if (!IsActive()) return;
	
	EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, false);
}

void UGA_SGSpanner::ApplyHitImpulse(ASGProjectileBase* Projectile, AActor* TargetActor)
{
	if (!IsValid(Projectile) || !IsValid(TargetActor)) return;
	
	ACharacter* TargetCharacter = Cast<ACharacter>(TargetActor);
	if (!TargetCharacter) return;
	
	UCharacterMovementComponent* MovementComponent = TargetCharacter->GetCharacterMovement();
	if (MovementComponent == nullptr) return;
	
	// Impulse 방향 계산
	FVector ImpulseDirection = (TargetActor->GetActorLocation() - Projectile->GetActorLocation()).GetSafeNormal();
	ImpulseDirection.Z = HitImpulseUpRatio;
	ImpulseDirection.Normalize();
	const FVector Impulse = ImpulseDirection * HitImpulseStrength;
	
	// Impulse 적용
	TargetCharacter->LaunchCharacter(Impulse, true, true);
}

bool UGA_SGSpanner::IsOtherTeam(AActor* TargetActor) const
{
	if (CurrentActorInfo == nullptr || !CurrentActorInfo->AvatarActor.IsValid() || !IsValid(TargetActor)) return false;
	
	APawn* OwnerPawn = Cast<APawn>(CurrentActorInfo->AvatarActor.Get());
	APawn* TargetPawn = Cast<APawn>(TargetActor);
	if (OwnerPawn == nullptr || TargetPawn == nullptr) return false;
	
	AController* OwnerController = OwnerPawn->GetController();
	AController* TargetController = TargetPawn->GetController();
	if (OwnerController == nullptr || TargetController == nullptr) return false;
	
	ASGMainPlayerState* OwnerPlayerState = OwnerController->GetPlayerState<ASGMainPlayerState>();
	ASGMainPlayerState* TargetPlayerState = TargetController->GetPlayerState<ASGMainPlayerState>();
	if (OwnerPlayerState == nullptr || TargetPlayerState == nullptr) return false;
	
	return OwnerPlayerState->CurrentTeamTag != TargetPlayerState->CurrentTeamTag;
}

void UGA_SGSpanner::ApplyDamageEffect(AActor* TargetActor)
{
	if (DamageEffectClass == nullptr || CurrentActorInfo == nullptr) return;
	
	UAbilitySystemComponent* OwnerASC = GetAbilitySystemComponentFromActorInfo();
	UAbilitySystemComponent* TargetASC = UAbilitySystemGlobals::GetAbilitySystemComponentFromActor(TargetActor);
	
	if (OwnerASC == nullptr || TargetASC == nullptr) return;
	
	// GE 적용 Context 생성
	FGameplayEffectContextHandle EffectContext = OwnerASC->MakeEffectContext();
	EffectContext.AddSourceObject(this);
	
	// 실제 적용할 Spec 생성
	FGameplayEffectSpecHandle EffectSpecHandle = OwnerASC->MakeOutgoingSpec(DamageEffectClass, 1.f, EffectContext);
	if (!EffectSpecHandle.IsValid()) return;
	
	// 생성한 Spec 적용
	OwnerASC->ApplyGameplayEffectSpecToTarget(*EffectSpecHandle.Data.Get(), TargetASC);
}
