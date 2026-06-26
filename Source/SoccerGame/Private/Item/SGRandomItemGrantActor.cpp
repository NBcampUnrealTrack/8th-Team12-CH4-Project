// Fill out your copyright notice in the Description page of Project Settings.


#include "Item/SGRandomItemGrantActor.h"

#include "Components/SphereComponent.h"
#include "Item/SGItemSlotComponent.h"
#include "Item/Data/SGItemDefinition.h"

// Sets default values
ASGRandomItemGrantActor::ASGRandomItemGrantActor(): bGranted(false)
{
 	PrimaryActorTick.bCanEverTick = false;

	bReplicates = true;
	SetReplicateMovement(false);
	
	Collision = CreateDefaultSubobject<USphereComponent>(TEXT("Collision"));
	SetRootComponent(Collision);
	
	Collision->InitSphereRadius(100.f);
	Collision->SetGenerateOverlapEvents(true);
	Collision->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	Collision->SetCollisionResponseToAllChannels(ECR_Ignore);
	Collision->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);
	Collision->OnComponentBeginOverlap.AddDynamic(this, &ASGRandomItemGrantActor::OnCollisionBeginOverlap);
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
	Collision->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	UE_LOG(LogGameplayTags, Warning, TEXT("지급된 아이템: %s"), *Item->GetName());
	Destroy();
}

USGItemDefinition* ASGRandomItemGrantActor::GetRandomItem()
{
	if (ItemPool.IsEmpty()) return nullptr;
	
	const int32 Index = FMath::RandRange(0, ItemPool.Num() - 1);
	
	return ItemPool[Index].Get();
}
