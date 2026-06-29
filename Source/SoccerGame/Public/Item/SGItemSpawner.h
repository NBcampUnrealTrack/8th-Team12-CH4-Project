// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "SGItemSpawner.generated.h"

class ASGRandomItemGrantActor;

UCLASS()
class SOCCERGAME_API ASGItemSpawner : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	ASGItemSpawner();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

private:
	void SpawnItem();
	
	FVector GetRandomSpawnLocation() const;
	
	UFUNCTION()
	void OnSpawnedItemGranted();
	
private:
	UPROPERTY(EditAnywhere, Category = "Item|Spawner")
	TSubclassOf<ASGRandomItemGrantActor> ItemClass;
	
	UPROPERTY(EditAnywhere, Category = "Item|Spawner")
	int32 SpawnCount;
	
	UPROPERTY(EditAnywhere, Category = "Item|Spawner")
	FVector SpawnExtent;
	
};
