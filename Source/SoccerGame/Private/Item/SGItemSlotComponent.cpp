// Fill out your copyright notice in the Description page of Project Settings.


#include "Item/SGItemSlotComponent.h"

#include "AbilitySystemComponent.h"
#include "Abilities/GameplayAbility.h"
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

void USGItemSlotComponent::UseItem_Implementation()
{
	if (ItemSlots.IsEmpty()) return;
	
	// 해당 아이템이 유효하고, 실행할 Gameplay Ability 확인
	USGItemDefinition* ItemDefinition = ItemSlots[0];
	if (!IsValid(ItemDefinition) || !ItemDefinition->AbilityClass) return;
	
	AActor* Owner = GetOwner();
	if (!IsValid(Owner) || !Owner->HasAuthority()) return;
	
	UAbilitySystemComponent* AbilitySystemComponent = Owner->FindComponentByClass<UAbilitySystemComponent>();
	if (!IsValid(AbilitySystemComponent)) return;
	
	// 아이템에 연결된 Gameplay Ability를 일회성 어빌리티로 생성
	FGameplayAbilitySpec AbilitySpec(
		ItemDefinition->AbilityClass, 1, INDEX_NONE, ItemDefinition);
	
	// 어빌리티를 ASC에 임시 부여하고 즉시 실행
	const FGameplayAbilitySpecHandle AbilityHandle = AbilitySystemComponent->GiveAbilityAndActivateOnce(AbilitySpec);
	if (!AbilityHandle.IsValid()) return;
	
	ItemSlots.RemoveAt(0);
	OnItemSlotChanged.Broadcast();
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
