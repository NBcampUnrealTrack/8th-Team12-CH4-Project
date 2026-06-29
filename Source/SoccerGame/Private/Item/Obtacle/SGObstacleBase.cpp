// Fill out your copyright notice in the Description page of Project Settings.


#include "Item/Obtacle/SGObstacleBase.h"

// Sets default values
ASGObstacleBase::ASGObstacleBase()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;
	bReplicates = true;
	
	MeshComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("MeshComponent"));
	SetRootComponent(MeshComponent);
	
	LifeTime = 5.f;
}

// Called when the game starts or when spawned
void ASGObstacleBase::BeginPlay()
{
	Super::BeginPlay();
	
	if (HasAuthority()) SetLifeSpan(LifeTime);
}
