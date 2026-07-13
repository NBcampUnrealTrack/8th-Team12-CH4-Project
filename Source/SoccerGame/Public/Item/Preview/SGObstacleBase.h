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

private:
	void UpdatePreviewTransform();

protected:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Obstacle")
	TObjectPtr<USceneComponent> SceneComponent;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Obstacle")
	TObjectPtr<UStaticMeshComponent> MeshComponent;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Obstacle")
	float LifeTime;
	
private:
	UPROPERTY()
	TObjectPtr<AActor> PreviewPlayerActor;
	
	float PreviewForwardDistance;
	
	float PreviewYawOffset;

	bool bPreview;
};
