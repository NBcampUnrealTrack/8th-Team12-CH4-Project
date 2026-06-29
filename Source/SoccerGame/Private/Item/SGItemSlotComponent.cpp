// Fill out your copyright notice in the Description page of Project Settings.


#include "Item/SGItemSlotComponent.h"

#include "Item/Data/SGItemDefinition.h"
#include "Net/UnrealNetwork.h"

// Sets default values for this component's properties
USGItemSlotComponent::USGItemSlotComponent() : MaxItemCount(2)
{
	PrimaryComponentTick.bCanEverTick = false;

	// 자체 복제 활성화
	SetIsReplicatedByDefault(true);
}

void USGItemSlotComponent::GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	
	// 변경 값을 해당 캐릭터를 조종하는 클라이언트에게만 복제
	DOREPLIFETIME_CONDITION(USGItemSlotComponent, ItemSlots, COND_OwnerOnly);
}

bool USGItemSlotComponent::AddItem(USGItemDefinition* NewItem)
{
	AActor* Owner = GetOwner();
	
	if (!IsValid(Owner) || !Owner->HasAuthority()) return false;
	// 아이템을 추가할 공간이 없을 경우
	if (!IsValid(NewItem) || ItemSlots.Num() >= MaxItemCount) return false;
	
	ItemSlots.Add(NewItem);
	OnItemSlotChanged.Broadcast();
	
	return true;
}

bool USGItemSlotComponent::ConsumeItem()
{
	AActor* Owner = GetOwner();
	
	if (!IsValid(Owner) || !Owner->HasAuthority()) return false;
	if (ItemSlots.IsEmpty()) return false;
	
	ItemSlots.RemoveAt(0);
	OnItemSlotChanged.Broadcast();
	
	return true;
}

int32 USGItemSlotComponent::GetItemCount() const
{
	return ItemSlots.Num();
}

USGItemDefinition* USGItemSlotComponent::GetItemAt(int32 Index) const
{
	return ItemSlots.IsValidIndex(Index) ? ItemSlots[Index].Get() : nullptr;
}

void USGItemSlotComponent::OnRep_ItemSlots()
{
	OnItemSlotChanged.Broadcast();
}
