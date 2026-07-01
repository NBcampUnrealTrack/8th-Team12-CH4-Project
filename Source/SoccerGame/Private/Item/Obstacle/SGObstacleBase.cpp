// Fill out your copyright notice in the Description page of Project Settings.


#include "Item/Obstacle/SGObstacleBase.h"

// Sets default values
ASGObstacleBase::ASGObstacleBase() : LifeTime(5.f), PreviewForwardDistance(500.f), bPreview(false)
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	PrimaryActorTick.SetTickFunctionEnable(false);
	bReplicates = true;
	
	MeshComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("MeshComponent"));
	SetRootComponent(MeshComponent);
}

// Called when the game starts or when spawned
void ASGObstacleBase::BeginPlay()
{
	Super::BeginPlay();
	
	if (!HasAuthority()) return;
		
	SetLifeSpan(LifeTime);
}

void ASGObstacleBase::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);
	
	if (!bPreview) return;
	
	UpdatePreviewTransform();
}

void ASGObstacleBase::InitializePreview(AActor* InPlayerActor, float InForwardDistance, float InPreviewOpacity)
{
	if (!IsValid(InPlayerActor)) return;
	
	PreviewPlayerActor = InPlayerActor;
	PreviewForwardDistance = InForwardDistance;
	bPreview = true;
	
	SetReplicates(false);
	SetActorEnableCollision(false);
	PrimaryActorTick.SetTickFunctionEnable(true);
	
	UpdatePreviewTransform();
}

void ASGObstacleBase::UpdatePreviewTransform()
{
	if (!IsValid(PreviewPlayerActor)) return;
	
	const FVector PreviewLocation = 
		PreviewPlayerActor->GetActorLocation() + PreviewPlayerActor->GetActorForwardVector() * PreviewForwardDistance;
	const FRotator PreviewRotation = PreviewPlayerActor->GetActorRotation();
	
	SetActorLocationAndRotation(PreviewLocation, PreviewRotation);
}
