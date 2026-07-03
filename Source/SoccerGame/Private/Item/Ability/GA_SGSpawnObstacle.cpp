// Fill out your copyright notice in the Description page of Project Settings.


#include "Item/Ability/GA_SGSpawnObstacle.h"

#include "Abilities/Tasks/AbilityTask_WaitInputRelease.h"
#include "Item/Obstacle/SGObstacleBase.h"

UGA_SGSpawnObstacle::UGA_SGSpawnObstacle() : SpawnForwardDistance(500.f), PreviewOpacity(0.35f)
{
	// 액터 별 인스턴스를 가짐
	InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;
	
	// 서버에서만 실행
	NetExecutionPolicy = EGameplayAbilityNetExecutionPolicy::LocalPredicted;
}

void UGA_SGSpawnObstacle::ActivateAbility(const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo,
	const FGameplayEventData* TriggerEventData)
{
	if (ActorInfo == nullptr || !ActorInfo->AvatarActor.IsValid()){
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}
	
	// 직접 조작하는 경우
	if (ActorInfo->IsLocallyControlled()){
		SpawnPreviewActor(ActorInfo);
	}
	
	UAbilityTask_WaitInputRelease* WaitInputReleaseTask =
		UAbilityTask_WaitInputRelease::WaitInputRelease(this, true);
	
	WaitInputReleaseTask->OnRelease.AddDynamic(this, &UGA_SGSpawnObstacle::OnInputReleased);
	WaitInputReleaseTask->ReadyForActivation();
}

void UGA_SGSpawnObstacle::OnInputReleased(float TimeHeld)
{
	// 액터 정보와 장애물 클래스가 유효한지 확인
	if (CurrentActorInfo == nullptr || !CurrentActorInfo->AvatarActor.IsValid() || ObstacleClass == nullptr){
		EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, true);
		return;
	}
	
	if (CurrentActorInfo->IsLocallyControlled()){
		DestroyPreviewActor();
	}
	
	// Spawn 작업은 서버에서만 처리
	AActor* PlayerActor = CurrentActorInfo->AvatarActor.Get();
	if (!PlayerActor->HasAuthority()){
		EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, false, false);
		return;
	}
	
	UWorld* World = GetWorld();
	if (World == nullptr){
		EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, true);
		return;
	}
	
	// 플레이어 전방에 장애물 생성
	const FVector SpawnLocation = PlayerActor->GetActorLocation() + PlayerActor->GetActorForwardVector() * SpawnForwardDistance;
	const FRotator SpawnRotation = PlayerActor->GetActorRotation();
	
	FActorSpawnParameters SpawnParams;
	SpawnParams.Owner = PlayerActor;
	SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
	
	ASGObstacleBase* SpawnedObstacle =
		World->SpawnActor<ASGObstacleBase>(ObstacleClass, SpawnLocation, SpawnRotation, SpawnParams);
	
	const bool bSpawnFailed = !IsValid(SpawnedObstacle);
	
	EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, bSpawnFailed, bSpawnFailed);
}

void UGA_SGSpawnObstacle::SpawnPreviewActor(const FGameplayAbilityActorInfo* ActorInfo)
{
	if (ObstacleClass == nullptr) return;
	
	UWorld* World = GetWorld();
	if (World == nullptr) return;
	
	DestroyPreviewActor();
	
	// 플레이어 전방에 Obstacle 생성
	AActor* PlayerActor = ActorInfo->AvatarActor.Get();
	const FVector PreviewLocation = PlayerActor->GetActorLocation() + PlayerActor->GetActorForwardVector() * SpawnForwardDistance;
	const FRotator PreviewRotation = PlayerActor->GetActorRotation();
	
	FActorSpawnParameters SpawnParams;
	SpawnParams.Owner = PlayerActor;
	SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
	
	PreviewActor =
		World->SpawnActor<ASGObstacleBase>(ObstacleClass, PreviewLocation, PreviewRotation, SpawnParams);
	if (!IsValid(PreviewActor)) return;
	
	PreviewActor->InitializePreview(PlayerActor, SpawnForwardDistance, PreviewOpacity);
}

void UGA_SGSpawnObstacle::DestroyPreviewActor()
{
	if (IsValid(PreviewActor)){
		PreviewActor->Destroy();
	}
	
	PreviewActor = nullptr;
}
