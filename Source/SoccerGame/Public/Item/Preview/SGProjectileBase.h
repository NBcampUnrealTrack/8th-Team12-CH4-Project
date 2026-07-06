// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "SGProjectileBase.generated.h"

class UProjectileMovementComponent;
class USphereComponent;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnProjectileHitTarget, ASGProjectileBase*, Projectile, AActor*, TargetActor);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnProjectileFinished, ASGProjectileBase*, Projectile);

UCLASS()
class SOCCERGAME_API ASGProjectileBase : public AActor
{
	GENERATED_BODY()
	
public:	
	ASGProjectileBase();
	virtual void Tick(float DeltaSeconds) override;

protected:
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
	
public:
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
	
	UPROPERTY(EditAnywhere, Category = "Projectile")
	float LifeTime;
	
	UPROPERTY(EditAnywhere, Category = "Projectile|Preview")
	float PreviewOpacity;
	
	// 발사체 설정
	float TargetDistance;
	float ThrowSpeed;
	
	// 생성 위치 설정
	float ThrowForwardOffset;
	float ThrowHeightOffset;
	
	// Preview 여부
	bool bPreview;
};
