// Fill out your copyright notice in the Description page of Project Settings.


#include "Item/Preview/SGProjectileBase.h"

#include "NiagaraFunctionLibrary.h"
#include "Components/AudioComponent.h"
#include "Components/SphereComponent.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Net/UnrealNetwork.h"

ASGProjectileBase::ASGProjectileBase() :  
	LifeTime(5.f),
	PreviewOpacity(0.6f),	
	TargetDistance(0.f), 
	ThrowSpeed(0.f), 
	ThrowForwardOffset(0.f), 
	ThrowHeightOffset(0.f), 
	bPreview(false),
	bDestroyOnSurface(true),
	Bounciness(0.35f),	
	EffectScale(1.f)
{
	// Tick 사용, 초깃값 비활성화
 	PrimaryActorTick.bCanEverTick = true;
	PrimaryActorTick.SetTickFunctionEnable(false);
	
	bReplicates = true;
	
	CollisionComponent = CreateDefaultSubobject<USphereComponent>(TEXT("CollisionComponent"));
	SetRootComponent(CollisionComponent);
	CollisionComponent->SetCollisionProfileName(TEXT("BlockAllDynamic"));
	CollisionComponent->OnComponentHit.AddDynamic(this, &ASGProjectileBase::OnProjectileHit);
	
	MeshComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("MeshComponent"));
	MeshComponent->SetupAttachment(CollisionComponent);
	MeshComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	MeshComponent->SetHiddenInGame(true);
	
	ProjectileMovement = CreateDefaultSubobject<UProjectileMovementComponent>(TEXT("ProjectileMovement"));
	ProjectileMovement->UpdatedComponent = CollisionComponent;
	ProjectileMovement->bAutoActivate = false;
	ProjectileMovement->bRotationFollowsVelocity = true;
	ProjectileMovement->bShouldBounce = true;
	ProjectileMovement->Bounciness = Bounciness;
}

void ASGProjectileBase::BeginPlay()
{
	Super::BeginPlay();
	
	// 서버(Released) -> LifeTime 적용
	if (HasAuthority()){
		SetLifeSpan(LifeTime);
	}
	else{
		// 판정용 투사체의 충돌 무시
		SetActorEnableCollision(false);
	}
}

void ASGProjectileBase::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	// 시각용 투사체 정리
	if (IsValid(ActiveCosmeticProjectile)){
		ActiveCosmeticProjectile->Destroy();
		ActiveCosmeticProjectile = nullptr;
	}
	
	if (!bPreview){
		if (GetNetMode() != NM_DedicatedServer){
			if (FinishedNiagaraEffect != nullptr){
				UNiagaraFunctionLibrary::SpawnSystemAtLocation(
					GetWorld(), FinishedNiagaraEffect, GetActorLocation(), GetActorRotation(), FVector(EffectScale));
			}else if (FinishedEffect != nullptr){
				UGameplayStatics::SpawnEmitterAtLocation(
					GetWorld(), FinishedEffect, GetActorLocation(), GetActorRotation(), FVector(EffectScale));	
			}
			
			if (FinishedSound != nullptr){
				UGameplayStatics::PlaySoundAtLocation(this, FinishedSound, GetActorLocation());
			}
		}
		
		OnProjectileFinished.Broadcast(this);
	}
	
	if (IsValid(FlightLoopAudioComponent)){
		FlightLoopAudioComponent->Stop();
		FlightLoopAudioComponent = nullptr;
	}
	Super::EndPlay(EndPlayReason);
}

void ASGProjectileBase::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);
	
	if (IsValid(MeshComponent) && GetLifeSpan() > 0.f &&LifeTime > KINDA_SMALL_NUMBER){
		const float Alpha = FMath::Clamp(GetGameTimeSinceCreation() / LifeTime, 0.f, 1.f);
		const float ChargeAmount = FMath::Lerp(-0.5f, 2.f, Alpha);
		MeshComponent->SetScalarParameterValueOnMaterials(TEXT("ExplosionChargeAmount"), ChargeAmount);
	}
	
	if (!bPreview) return;
	
	FVector StartLocation, LaunchVelocity;
	FHitResult HitResult;

	// 충돌 예측 지점으로 배치
	if (CalculateTrajectory(StartLocation, LaunchVelocity, HitResult)){
		SetActorLocation(HitResult.Location);
	}
}

void ASGProjectileBase::GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	
	DOREPLIFETIME(ASGProjectileBase, CosmeticLaunchData);
}

void ASGProjectileBase::InitializeCosmeticProjectile(const FVector& StartLocation, const FVector& LaunchVelocity)
{
	bPreview = true;
	PrimaryActorTick.SetTickFunctionEnable(true);
	
	// 복제 비활성화
	SetReplicates(false);
	SetLifeSpan(LifeTime);
	
	// 바닥, 벽을 대상을 충돌 활성화
	SetActorEnableCollision(true);
	CollisionComponent->SetCollisionResponseToAllChannels(ECR_Ignore);
	CollisionComponent->SetCollisionResponseToChannel(ECC_WorldStatic, ECR_Block);
	CollisionComponent->SetCollisionResponseToChannel(ECC_WorldDynamic, ECR_Block);
	
	// 해당 투사체가 보이도록 변경
	if (IsValid(MeshComponent)){
		MeshComponent->SetHiddenInGame(false);
	}
	
	// 생성위치로 배치
	SetActorLocationAndRotation(StartLocation, LaunchVelocity.Rotation());
	
	// Velocity 적용 및 Movement 활성화
	ProjectileMovement->Velocity = LaunchVelocity;
	ProjectileMovement->Activate(true);
	
	if (FlightLoopSound != nullptr){
		FlightLoopAudioComponent = UGameplayStatics::SpawnSoundAttached(
			FlightLoopSound, GetRootComponent());
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
		MeshComponent->SetHiddenInGame(false);
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
	
	// 자기 자신과의 충돌 무시
	CollisionComponent->IgnoreActorWhenMoving(PlayerActor, true);
	if (UPrimitiveComponent* PlayerRootComponent = Cast<UPrimitiveComponent>(PlayerActor->GetRootComponent())){
		PlayerRootComponent->IgnoreActorWhenMoving(this, true);
	}
	
	// 충돌 예측 지점 계산
	FVector StartLocation, LaunchVelocity;
	FHitResult HitResult;
	if (!CalculateTrajectory(StartLocation, LaunchVelocity, HitResult)) return false;
	
	if (HasAuthority()){
		CosmeticLaunchData.StartLocation = StartLocation;
		CosmeticLaunchData.LaunchVelocity = LaunchVelocity;
		
		if (GetNetMode() != NM_DedicatedServer){
			SpawnCosmeticProjectile();
		}
	}
	
	// 발사 시작 위치와 이동 방향 적용
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
	PathParams.TraceChannel = ECC_Pawn;
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

void ASGProjectileBase::OnProjectileHit(UPrimitiveComponent* HitComponent, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit)
{
	if (!HasAuthority() || bPreview) return;
	if (!IsValid(OtherActor) || OtherActor == this || OtherActor == PlayerActor) return;
	
	// 바닥, 벽에 충돌했을 때의 처리
	const bool bHitPawn = OtherActor->IsA<APawn>();
	if (!bHitPawn && !bDestroyOnSurface) return;
	
	OnProjectileHitTarget.Broadcast(this, OtherActor);
	
	if (bHitPawn){
		// 플레이어 타격 여부는 서버 Hit 판정에서만 확정되므로, 이 시점에 모든 클라이언트로 사운드 재생을 전달
		MulticastPlayHitTargetSound(GetActorLocation());
	}
	
	Destroy();
}

void ASGProjectileBase::MulticastPlayHitTargetSound_Implementation(FVector SoundLocation)
{
	if (GetNetMode() == NM_DedicatedServer || HitTargetSound == nullptr) return;
	
	// Spanner처럼 플레이어 타격 시에만 필요한 사운드 재생
	UGameplayStatics::PlaySoundAtLocation(this, HitTargetSound, SoundLocation);
}

void ASGProjectileBase::OnRep_CosmeticLaunchData()
{
	SpawnCosmeticProjectile();
}

void ASGProjectileBase::SpawnCosmeticProjectile()
{
	if (CosmeticLaunchData.LaunchVelocity.IsNearlyZero()) return;
	if (IsValid(ActiveCosmeticProjectile)) return;
	
	UWorld* World = GetWorld();
	if (!IsValid(World)) return;
	
	FActorSpawnParameters SpawnParams;
	SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
	
	// 시각용 발사체 생성
	ActiveCosmeticProjectile = World->SpawnActor<ASGProjectileBase>(
		GetClass(), CosmeticLaunchData.StartLocation, CosmeticLaunchData.LaunchVelocity.Rotation(), SpawnParams);
	if (!IsValid(ActiveCosmeticProjectile)) return;
	
	// Player - Projectile 충돌 무시
	CollisionComponent->IgnoreActorWhenMoving(ActiveCosmeticProjectile, true);
	ActiveCosmeticProjectile->CollisionComponent->IgnoreActorWhenMoving(this, true);
	
	ActiveCosmeticProjectile->InitializeCosmeticProjectile(
		CosmeticLaunchData.StartLocation, CosmeticLaunchData.LaunchVelocity);
}
