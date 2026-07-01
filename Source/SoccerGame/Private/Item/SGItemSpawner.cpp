// Fill out your copyright notice in the Description page of Project Settings.


#include "Item/SGItemSpawner.h"

#include "Item/SGRandomItemGrantActor.h"

// Sets default values
ASGItemSpawner::ASGItemSpawner(): SpawnCount(0), SpawnExtent(FVector::ZeroVector)
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;

}

// Called when the game starts or when spawned
void ASGItemSpawner::BeginPlay()
{
	Super::BeginPlay();
	
	if (!HasAuthority() || !ItemClass) return;
	
	for (int32 i = 0; i< SpawnCount; ++i){
		SpawnItem();
	}
}

void ASGItemSpawner::SpawnItem()
{
	if (!HasAuthority() || !ItemClass) return;
	
	UWorld* World = GetWorld();
	if (!IsValid(World)) return;
	
	// 생성 위치
	FVector SpawnLocation = GetRandomSpawnLocation();
	
	// 충돌하지 않는 경우에만 생성
	FActorSpawnParameters SpawnParams;
	SpawnParams.Owner = this;
	SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::DontSpawnIfColliding;
	
	ASGRandomItemGrantActor* SpawnedItem = nullptr;
	
	// 다른 오브젝트와 겹치는 경우 위로 10회 보정하며 시도
	int32 MaxHeightAdjustCount = 10;
	int32 HeightAdjustStep = 10;
	for (int32 i = 0; i < MaxHeightAdjustCount; ++i){
		SpawnedItem = World->SpawnActor<ASGRandomItemGrantActor>(
			ItemClass, SpawnLocation, FRotator::ZeroRotator, SpawnParams);
		
		if (IsValid(SpawnedItem)) break;
		
		SpawnLocation.Z += HeightAdjustStep;
	}
	
	// 10회의 보정 작업 이후에도 생성되지 않으면 자동으로 보정하도록 해서 생성
	if (!IsValid(SpawnedItem)){
		SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;
		
		SpawnedItem = World->SpawnActor<ASGRandomItemGrantActor>(
			ItemClass, SpawnLocation, FRotator::ZeroRotator, SpawnParams);
	}
	
	if (!IsValid(SpawnedItem)) return;
	
	// 삭제(아이템 획득) 델리게이트 등록 
	SpawnedItem->OnRandomItemGranted.AddDynamic(this, &ASGItemSpawner::OnSpawnedItemGranted);
}

FVector ASGItemSpawner::GetRandomSpawnLocation() const
{
	const FVector RandomOffset(
		FMath::RandRange(-SpawnExtent.X, SpawnExtent.X),
		FMath::RandRange(-SpawnExtent.Y, SpawnExtent.Y),
		0.f
	);
	
	return GetActorLocation() + RandomOffset;
}

void ASGItemSpawner::OnSpawnedItemGranted()
{
	if (!HasAuthority()) return;
	
	SpawnItem();
}

