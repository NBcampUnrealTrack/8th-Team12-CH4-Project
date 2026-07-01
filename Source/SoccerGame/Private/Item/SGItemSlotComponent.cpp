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

void USGItemSlotComponent::UseItemPressed()
{
	if (ItemSlots.IsEmpty()) return;
	
	// 이전 AbilitySpecHandle 무효화
	ActiveItemAbilityHandle = FGameplayAbilitySpecHandle();
	
	USGItemDefinition* ItemDefinition = ItemSlots[0];
	if (!IsValid(ItemDefinition) || !ItemDefinition->AbilityClass) return;
	
	AActor* Owner = GetOwner();
	if (!IsValid(Owner) || !Owner->HasAuthority()) return;
	
	UAbilitySystemComponent* AbilitySystemComponent = Owner->FindComponentByClass<UAbilitySystemComponent>();
	if (!IsValid(AbilitySystemComponent)) return;
	
	// GA을 ASC에 일회성으로 등록
	FGameplayAbilitySpec AbilitySpec(ItemDefinition->AbilityClass, 1, INDEX_NONE, ItemDefinition);
	FGameplayAbilitySpecHandle AbilityHandle = AbilitySystemComponent->GiveAbilityAndActivateOnce(AbilitySpec);
	if (!AbilityHandle.IsValid()) return;
	
	// Released 입력에서 InputReleased을 호출하기 위해 보관
	ActiveItemAbilityHandle = AbilityHandle;
}

void USGItemSlotComponent::UseItemReleased_Implementation()
{
	if (ItemSlots.IsEmpty()) return;
	
	// 해당 아이템이 유효한지, 실행할 Gameplay Ability가 있는지
	USGItemDefinition* ItemDefinition = ItemSlots[0];
	if (!IsValid(ItemDefinition) || !ItemDefinition->AbilityClass) return;
	
	AActor* Owner = GetOwner();
	if (!IsValid(Owner) || !Owner->HasAuthority()) return;
	
	UAbilitySystemComponent* AbilitySystemComponent = Owner->FindComponentByClass<UAbilitySystemComponent>();
	if (!IsValid(AbilitySystemComponent) || !ActiveItemAbilityHandle.IsValid()) return;
	
	FGameplayAbilitySpec* AbilitySpec =
		AbilitySystemComponent->FindAbilitySpecFromHandle(ActiveItemAbilityHandle);
	if (AbilitySpec == nullptr) return;
	
	
	AbilitySystemComponent->AbilitySpecInputReleased(*AbilitySpec);
	
	// 아이템 사용 성공 여부와는 관계 없이 소모 처리
	ItemSlots.RemoveAt(0);
	OnItemSlotChanged.Broadcast();
	ActiveItemAbilityHandle = FGameplayAbilitySpecHandle();
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
