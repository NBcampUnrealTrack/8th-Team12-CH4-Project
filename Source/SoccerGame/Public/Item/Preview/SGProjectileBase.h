// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "SGProjectileBase.generated.h"

class UProjectileMovementComponent;
class USphereComponent;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnProjectileHitTarget, ASGProjectileBase*, Projectile, AActor*, TargetActor);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnProjectileFinished, ASGProjectileBase*, Projectile);

// 발사체 발사 정보
USTRUCT()
struct FSGProjectileCosmeticLaunchData
{
	GENERATED_BODY()
	
	UPROPERTY()
	FVector StartLocation = FVector::ZeroVector;
	
	UPROPERTY()
	FVector LaunchVelocity = FVector::ZeroVector;
};

UCLASS()
class SOCCERGAME_API ASGProjectileBase : public AActor
{
	GENERATED_BODY()
	
public:	
	ASGProjectileBase();
	virtual void Tick(float DeltaSeconds) override;
	virtual void GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const override;
	
protected:
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
	
public:
	// local에서 생성할 시각용 발사체 설정
	void InitializeCosmeticProjectile(const FVector& StartLocation, const FVector& LaunchVelocity);
	
	// Preview 초기 설정
	void InitializePreview(AActor* InPlayerActor,
		float InTargetDistance,
		float InThrowSpeed,
		float InThrowForwardOffset,
		float InThrowHeightOffset);
	
	// 계산된 궤적으로 발사체 발사
	bool LaunchByTrajectory(
		AActor* InPlayerActor,
		float InTargetDistance,
		float InThrowSpeed,
		float InThrowForwardOffset,
		float InThrowHeightOffset);
	
private:
	// 발사체 충돌 예측 지점 계산
	bool CalculateTrajectory(FVector& OutStartLocation, FVector& OutLaunchVelocity, FHitResult& OutHitResult) const;
	
	// 발사체 충돌
	UFUNCTION()
	void OnProjectileHit(
		UPrimitiveComponent* HitComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp,
		FVector NormalImpulse, const FHitResult& Hit);
	
	// 시각용 발사체 생성
	UFUNCTION()
	void OnRep_CosmeticLaunchData();
	
	void SpawnCosmeticProjectile();
	
public:
	// 델리게이트
	UPROPERTY()
	FOnProjectileHitTarget OnProjectileHitTarget;
	
	UPROPERTY()
	FOnProjectileFinished OnProjectileFinished;
	
private:
	UPROPERTY(VisibleAnywhere, Category = "Projectile")
	TObjectPtr<USphereComponent> CollisionComponent;
	
	UPROPERTY(EditAnywhere, Category = "Projectile")
	TObjectPtr<UStaticMeshComponent> MeshComponent;
	
	UPROPERTY(VisibleAnywhere, Category = "Projectile")
	TObjectPtr<UProjectileMovementComponent> ProjectileMovement;
	
	UPROPERTY()
	TObjectPtr<AActor> PlayerActor;
	
	// 최대 유지 시간
	UPROPERTY(EditAnywhere, Category = "Projectile")
	float LifeTime;
	
	// Preview 투명도
	UPROPERTY(EditAnywhere, Category = "Projectile|Preview")
	float PreviewOpacity;
	
	// 시각용 발사체
	UPROPERTY(ReplicatedUsing = OnRep_CosmeticLaunchData)
	FSGProjectileCosmeticLaunchData CosmeticLaunchData;
	
	UPROPERTY()
	TObjectPtr<ASGProjectileBase> ActiveCosmeticProjectile;
	
	// 발사체 설정
	float TargetDistance;
	float ThrowSpeed;
	
	// 생성 위치 설정
	float ThrowForwardOffset;
	float ThrowHeightOffset;
	
	// Preview 여부
	bool bPreview;
};
