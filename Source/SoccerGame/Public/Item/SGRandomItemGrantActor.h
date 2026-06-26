// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "SGRandomItemGrantActor.generated.h"

class USGItemDefinition;
class USphereComponent;

UCLASS()
class SOCCERGAME_API ASGRandomItemGrantActor : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	ASGRandomItemGrantActor();

private:
	UFUNCTION()
	void OnCollisionBeginOverlap(
		UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComponent, 
		int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);
	
	USGItemDefinition* GetRandomItem();

private:
	UPROPERTY(VisibleAnywhere, Category = "Item")
	TObjectPtr<USphereComponent> Collision;
	
	UPROPERTY(EditAnywhere, Category = "Item")
	TArray<TObjectPtr<USGItemDefinition>> ItemPool;
	
	bool bGranted;
};
