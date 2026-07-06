// Fill out your copyright notice in the Description page of Project Settings.


#include "Item/Preview/SGProjectileBase.h"

#include "Components/SphereComponent.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "Kismet/GameplayStatics.h"

ASGProjectileBase::ASGProjectileBase() :  
	TargetDistance(0.f), 
	ThrowSpeed(0.f), 
	ThrowForwardOffset(0.f), 
	ThrowHeightOffset(0.f), 
	bPreview(false),
	LifeTime(5.f),
	PreviewOpacity(0.35f)
{
	// Tick 사용, 초깃값 비활성화
 	PrimaryActorTick.bCanEverTick = true;
	PrimaryActorTick.SetTickFunctionEnable(false);
	
	// Movement 복제 활성화
	bReplicates = true;
	SetReplicateMovement(true);
	
	CollisionComponent = CreateDefaultSubobject<USphereComponent>(TEXT("CollisionComponent"));
	SetRootComponent(CollisionComponent);
	CollisionComponent->SetCollisionProfileName(TEXT("BlockAllDynamic"));
	
	MeshComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("MeshComponent"));
	MeshComponent->SetupAttachment(CollisionComponent);
	MeshComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	
	ProjectileMovement = CreateDefaultSubobject<UProjectileMovementComponent>(TEXT("ProjectileMovement"));
	ProjectileMovement->UpdatedComponent = CollisionComponent;
	ProjectileMovement->bAutoActivate = false;
	ProjectileMovement->bRotationFollowsVelocity = true;
}

void ASGProjectileBase::BeginPlay()
{
	Super::BeginPlay();
	
	// 서버(Released) -> LifeTime 적용
	if (HasAuthority()){
		SetLifeSpan(LifeTime);
	}
}

void ASGProjectileBase::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);
	
	if (!bPreview) return;
	
	FVector StartLocation, LaunchVelocity;
	FHitResult HitResult;

	// 충돌 예측 지점으로 배치
	if (CalculateTrajectory(StartLocation, LaunchVelocity, HitResult)){
		SetActorLocation(HitResult.Location);
	}
}

void ASGProjectileBase::InitializePreview(AActor* InPlayerActor, float InTargetDistance, float InThrowSpeed,
                                          float InThrowForwardOffset, float InThrowHeightOffset)
{
	if (!IsValid(InPlayerActor)) return;

	// 발사체 계산에 사용할 Actor와 발사 설정 저장
	PlayerActor = InPlayerActor;
	TargetDistance = InTargetDistance;
	ThrowSpeed = InThrowSpeed;
	ThrowForwardOffset = InThrowForwardOffset;
	ThrowHeightOffset = InThrowHeightOffset;
	bPreview = true;
	
	SetLifeSpan(0.f);
	SetReplicates(false);
	SetActorEnableCollision(false);
	
	// 투명도 적용
	if (IsValid(MeshComponent)){
		for (int32 Index = 0; Index < MeshComponent->GetNumMaterials(); ++Index){
			UMaterialInstanceDynamic* DynamicMaterial = MeshComponent->CreateDynamicMaterialInstance(Index);
			if (!IsValid(DynamicMaterial)) continue;
			
			DynamicMaterial->SetScalarParameterValue(TEXT("Opacity"), PreviewOpacity);
		}
	}
	
	// Tick 활성화
	PrimaryActorTick.SetTickFunctionEnable(true);
}

bool ASGProjectileBase::LaunchByTrajectory(AActor* InPlayerActor, float InTargetDistance, float InThrowSpeed,
	float InThrowForwardOffset, float InThrowHeightOffset)
{
	if (!IsValid(InPlayerActor)) return false;

	// 발사체 계산에 사용할 Actor와 발사 설정 저장
	PlayerActor = InPlayerActor;
	TargetDistance = InTargetDistance;
	ThrowSpeed = InThrowSpeed;
	ThrowForwardOffset = InThrowForwardOffset;
	ThrowHeightOffset = InThrowHeightOffset;
	bPreview = false;
	
	// 충돌 예측 지점 계산
	FVector StartLocation, LaunchVelocity;
	FHitResult HitResult;
	if (!CalculateTrajectory(StartLocation, LaunchVelocity, HitResult)) return false;
	
	// 시작 지점과 Rotation
	SetActorLocationAndRotation(StartLocation, LaunchVelocity.Rotation());
	
	// Velocity 적용 및 movement 활성화
	ProjectileMovement->Velocity = LaunchVelocity;
	ProjectileMovement->Activate(true);
	
	return true;
}

bool ASGProjectileBase::CalculateTrajectory(
	FVector& OutStartLocation, FVector& OutLaunchVelocity, FHitResult& OutHitResult) const
{
	if (!IsValid(PlayerActor)) return false;
	
	// 발사 및 목표 위치 계산
	OutStartLocation = PlayerActor->GetActorLocation()
		+ PlayerActor->GetActorForwardVector() * ThrowForwardOffset
		+ FVector(0.f, 0.f, ThrowHeightOffset);
	FVector AimTargetLocation = PlayerActor->GetActorLocation()
		+ PlayerActor->GetActorForwardVector() * TargetDistance;
	
	// 목표 위치까지 throwSpeed로 도달할 수 있는 초기 속도 계산
	const bool bFoundVelocity = UGameplayStatics::SuggestProjectileVelocity(
		this, 
		OutLaunchVelocity, 
		OutStartLocation, 
		AimTargetLocation,
		ThrowSpeed,
		false,
		0.f,
		0.f,
		ESuggestProjVelocityTraceOption::DoNotTrace
	);
	
	if (!bFoundVelocity) return false;
	
	// 구한 초기 속도로 시뮬레이션
	FPredictProjectilePathParams PathParams;
	PathParams.StartLocation = OutStartLocation;
	PathParams.LaunchVelocity = OutLaunchVelocity;
	PathParams.ProjectileRadius = CollisionComponent->GetScaledSphereRadius();
	PathParams.MaxSimTime = LifeTime;
	PathParams.TraceChannel = ECC_Visibility;
	PathParams.bTraceWithCollision = true;
	PathParams.ActorsToIgnore.Add(PlayerActor);

	FPredictProjectilePathResult PathResult;
	const bool bHit = UGameplayStatics::PredictProjectilePath(this, PathParams, PathResult);
	
	// 충돌 -> 결과 반환
	if (bHit){
		OutHitResult = PathResult.HitResult;
		return true;
	}
	
	// 충돌 X -> 경로의 마지막 위치 저장하여 반환
	if (!PathResult.PathData.IsEmpty()){
		OutHitResult.Location = PathResult.PathData.Last().Location;
		return true;
	}
	
	return false;
}
