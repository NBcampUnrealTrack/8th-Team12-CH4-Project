// Fill out your copyright notice in the Description page of Project Settings.


#include "Item/Obstacle/SGObstacleBase.h"

// Sets default values
ASGObstacleBase::ASGObstacleBase() : LifeTime(5.f), PreviewForwardDistance(500.f), bPreview(false)
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	PrimaryActorTick.SetTickFunctionEnable(false);
	bReplicates = true;
	
	SceneComponent = CreateDefaultSubobject<USceneComponent>(TEXT("SceneRootComponent"));
	SetRootComponent(SceneComponent);
	
	MeshComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("MeshComponent"));
	MeshComponent->SetupAttachment(SceneComponent);
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
	
	// LifsSpan 취소
	SetLifeSpan(0.f);
	// 네트워크로 복제하지 않도록 설정
	SetReplicates(false);
	// 충돌 무시
	SetActorEnableCollision(false);
	// Tick 활성화
	PrimaryActorTick.SetTickFunctionEnable(true);
	
	UpdatePreviewTransform();
	
	// Matreial의 투명도 설정
	// RootComponent 하위에 여러개의 Mesh가 있는 경우(임시)
	TArray<UStaticMeshComponent*> StaticMeshComponents;
	GetComponents<UStaticMeshComponent>(StaticMeshComponents);
	
	for (UStaticMeshComponent* StaticMeshComponent : StaticMeshComponents){
		if (!IsValid(StaticMeshComponent)) continue;
		
		for (int32 i = 0; i < StaticMeshComponent->GetNumMaterials(); ++i){
			UMaterialInstanceDynamic* DynamicMaterial = StaticMeshComponent->CreateDynamicMaterialInstance(i);
			if (!IsValid(DynamicMaterial)) continue;
			
			DynamicMaterial->SetScalarParameterValue(TEXT("Opacity"), InPreviewOpacity);
		}
	}
	
	/*
	단일 Mesh로 변경 시 사용
	if (!IsValid(MeshComponent)) return;
	for (int32 Index = 0; Index < MeshComponent->GetNumMaterials(); ++Index){
		UMaterialInstanceDynamic* DynamicMaterial = MeshComponent->CreateDynamicMaterialInstance(Index);
		if (!IsValid(DynamicMaterial)) continue;
	
		DynamicMaterial->SetScalarParameterValue(TEXT("Opacity"), InPreviewOpacity);
	}
	*/
}

void ASGObstacleBase::UpdatePreviewTransform()
{
	if (!IsValid(PreviewPlayerActor)) return;
	
	const FVector PreviewLocation = 
		PreviewPlayerActor->GetActorLocation() + PreviewPlayerActor->GetActorForwardVector() * PreviewForwardDistance;
	const FRotator PreviewRotation = PreviewPlayerActor->GetActorRotation();
	
	SetActorLocationAndRotation(PreviewLocation, PreviewRotation);
}
