// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "SGObstacleBase.generated.h"

UCLASS()
class SOCCERGAME_API ASGObstacleBase : public AActor
{
	GENERATED_BODY()
	
public:	
	ASGObstacleBase();

protected:
	virtual void BeginPlay() override;
	
	virtual void Tick(float DeltaSeconds) override;
	
public:
	void InitializePreview(AActor* InPlayerActor, float InForwardDistance, float InPreviewOpacity);
	
	void SetPreviewYawOffset(float InYawOffset);
	
	void PlaySpawnSound();

private:
	void UpdatePreviewTransform();
	
	UFUNCTION(NetMulticast, Unreliable)
	void MulticastPlaySpawnSound(FVector SoundLocation);

protected:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Obstacle")
	TObjectPtr<USceneComponent> SceneComponent;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Obstacle")
	TObjectPtr<UStaticMeshComponent> MeshComponent;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Obstacle")
	float LifeTime;
	
	UPROPERTY(EditAnywhere, Category = "Obstacle|Sound")
	TObjectPtr<USoundBase> SpawnSound;
	
private:
	UPROPERTY()
	TObjectPtr<AActor> PreviewPlayerActor;
	
	float PreviewForwardDistance;
	
	float PreviewYawOffset;

	bool bPreview;
};
