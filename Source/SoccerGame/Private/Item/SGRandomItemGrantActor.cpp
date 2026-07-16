// Fill out your copyright notice in the Description page of Project Settings.


#include "Item/SGRandomItemGrantActor.h"

#include "NiagaraFunctionLibrary.h"
#include "Components/BoxComponent.h"
#include "Components/SceneComponent.h"
#include "Item/SGItemSlotComponent.h"
#include "Item/Data/SGItemDefinition.h"
#include "Item/Visual/SGRandomItemGrantVisualComponent.h"
#include "Kismet/GameplayStatics.h"

// Sets default values
ASGRandomItemGrantActor::ASGRandomItemGrantActor(): bGranted(false)
{
 	PrimaryActorTick.bCanEverTick = false;

	bReplicates = true;

	SceneRoot = CreateDefaultSubobject<USceneComponent>(TEXT("SceneRoot"));
	SetRootComponent(SceneRoot);
	
	Collision = CreateDefaultSubobject<UBoxComponent>(TEXT("Collision"));
	Collision->SetupAttachment(SceneRoot);
	
	Collision->InitBoxExtent(FVector(80.f, 80.f, 100.f));
	Collision->SetRelativeLocation(FVector(0.f, 0.f, 100.f));
	Collision->SetGenerateOverlapEvents(true);
	Collision->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	Collision->SetCollisionResponseToAllChannels(ECR_Ignore);
	Collision->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);
	Collision->OnComponentBeginOverlap.AddDynamic(this, &ASGRandomItemGrantActor::OnCollisionBeginOverlap);

	VisualComponent = CreateDefaultSubobject<USGRandomItemGrantVisualComponent>(TEXT("VisualComponent"));
	VisualComponent->SetupAttachment(SceneRoot);
	
}

void ASGRandomItemGrantActor::OnCollisionBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
	UPrimitiveComponent* OtherComponent, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if (!HasAuthority() || !IsValid(OtherActor) || OtherActor == this) return;
	// 다른 플레이어가 이미 획득
	if (bGranted) return;
	
	USGItemSlotComponent* ItemSlotComponent = OtherActor->FindComponentByClass<USGItemSlotComponent>();
	if (!IsValid(ItemSlotComponent)) return;
	
	USGItemDefinition* Item = GetRandomItem();
	if (!IsValid(Item)) return;
	
	// 아이템 슬롯에 들어가는 것이 실패한 경우 false이 반환
	if (!ItemSlotComponent->AddItem(Item)) return;
	
	bGranted = true;
	if (IsValid(OverlappedComponent)){
		OverlappedComponent->SetGenerateOverlapEvents(false);
		OverlappedComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	}else if (IsValid(Collision)){
		Collision->SetGenerateOverlapEvents(false);
		Collision->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	}
	
	MulticastPickup(GetActorLocation());
	
	OnRandomItemGranted.Broadcast();
	Destroy();
}

void ASGRandomItemGrantActor::MulticastPickup_Implementation(FVector ActorLocation)
{
	if (GetNetMode() == NM_DedicatedServer) return;
	
	if (PickupEffect != nullptr){
		UNiagaraFunctionLibrary::SpawnSystemAtLocation(
        		GetWorld(), PickupEffect, ActorLocation, GetActorRotation());	
	}
	
	if(PickupSound != nullptr){
		UGameplayStatics::PlaySoundAtLocation(this, PickupSound, ActorLocation);
	}
}

USGItemDefinition* ASGRandomItemGrantActor::GetRandomItem()
{
	if (ItemPool.IsEmpty()) return nullptr;
	
	const int32 Index = FMath::RandRange(0, ItemPool.Num() - 1);
	
	return ItemPool[Index].Get();
}
