// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "SGRandomItemGrantActor.generated.h"

class UNiagaraSystem;
class USGItemDefinition;
class UBoxComponent;
class USceneComponent;
class USGRandomItemGrantVisualComponent;

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnRandomItemGranted);

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
	
	UFUNCTION(NetMulticast, Reliable)
	void MulticastPickup(FVector ActorLocation);
	
	USGItemDefinition* GetRandomItem();

public:
	UPROPERTY(BlueprintAssignable, Category = "Item")
	FOnRandomItemGranted OnRandomItemGranted;
	
private:
	UPROPERTY(VisibleAnywhere, Category = "Item")
	TObjectPtr<USceneComponent> SceneRoot;

	UPROPERTY(VisibleAnywhere, Category = "Item")
	TObjectPtr<UBoxComponent> Collision;	
	
	UPROPERTY(EditAnywhere, Category = "Item")
	TArray<TObjectPtr<USGItemDefinition>> ItemPool;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Item|Visual", meta=(AllowPrivateAccess="true"))
	TObjectPtr<USGRandomItemGrantVisualComponent> VisualComponent;
	
	UPROPERTY(EditAnywhere, Category = "Item")
	TObjectPtr<UNiagaraSystem> PickupEffect;
	
	UPROPERTY(EditAnywhere, Category = "Item")
	TObjectPtr<USoundBase> PickupSound; 
	
	bool bGranted;
};
