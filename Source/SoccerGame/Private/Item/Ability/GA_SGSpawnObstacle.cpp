// Fill out your copyright notice in the Description page of Project Settings.


#include "Item/Ability/GA_SGSpawnObstacle.h"

#include "Item/Obstacle/SGObstacleBase.h"

UGA_SGSpawnObstacle::UGA_SGSpawnObstacle()
{
	// 액터 별 인스턴스를 가짐
	InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;
	
	// 서버에서만 실행
	NetExecutionPolicy = EGameplayAbilityNetExecutionPolicy::ServerOnly;
}

void UGA_SGSpawnObstacle::ActivateAbility(const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo,
	const FGameplayEventData* TriggerEventData)
{
	// 액터 정보와 장애물 클래스가 유효한지 확인
	if (ActorInfo == nullptr || !ActorInfo->AvatarActor.IsValid() || ObstacleClass == nullptr){
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;	
	}
	
	// Spawn 작업은 서버에서만 처리
	AActor* PlayerActor = ActorInfo->AvatarActor.Get();
	if (!PlayerActor->HasAuthority()){
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}
	
	UWorld* World = GetWorld();
	if (World == nullptr){
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}
	
	// 플레이어 전방에 장애물 생성
	const FVector SpawnLocation = PlayerActor->GetActorLocation() + PlayerActor->GetActorForwardVector() * 500.f;
	const FRotator SpawnRotation = PlayerActor->GetActorRotation();
	
	FActorSpawnParameters SpawnParams;
	SpawnParams.Owner = PlayerActor;
	SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
	
	ASGObstacleBase* SpawnedObstacle = 
		World->SpawnActor<ASGObstacleBase>(ObstacleClass, SpawnLocation, SpawnRotation, SpawnParams);
	
	const bool bSpawnFailed = !IsValid(SpawnedObstacle);
	
	EndAbility(Handle, ActorInfo, ActivationInfo, bSpawnFailed, bSpawnFailed);
}
