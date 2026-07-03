// Fill out your copyright notice in the Description page of Project Settings.


#include "Item/SGItemSlotComponent.h"

#include "AbilitySystemComponent.h"
#include "Abilities/GameplayAbility.h"
#include "GameFramework/Pawn.h"
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
	DOREPLIFETIME_CONDITION(USGItemSlotComponent, ItemAbilityHandles, COND_OwnerOnly);
}

bool USGItemSlotComponent::AddItem(USGItemDefinition* NewItem)
{
	AActor* Owner = GetOwner();
	if (!IsValid(Owner) || !Owner->HasAuthority()) return false;
	// 아이템을 추가할 공간이 없을 경우
	if (!IsValid(NewItem) || ItemSlots.Num() >= MaxItemCount) return false;
	if (!NewItem->AbilityClass) return false;
	
	UAbilitySystemComponent* AbilitySystemComponent = Owner->FindComponentByClass<UAbilitySystemComponent>();
	if (!IsValid(AbilitySystemComponent)) return false;
	
	FGameplayAbilitySpec AbilitySpec(NewItem->AbilityClass, 1, INDEX_NONE, NewItem);
	FGameplayAbilitySpecHandle AbilityHandle = AbilitySystemComponent->GiveAbility(AbilitySpec);
	if (!AbilityHandle.IsValid()) return false;
	
	ItemSlots.Add(NewItem);
	ItemAbilityHandles.Add(AbilityHandle);
	OnItemSlotChanged.Broadcast();
	
	return true;
}

void USGItemSlotComponent::UseItemPressed()
{
	if (ItemSlots.IsEmpty() || !ItemAbilityHandles.IsValidIndex(0)) return;
	
	USGItemDefinition* ItemDefinition = ItemSlots[0];
	if (!IsValid(ItemDefinition) || !ItemDefinition->AbilityClass) return;
	
	AActor* Owner = GetOwner();
	if (!IsValid(Owner)) return;
	
	UAbilitySystemComponent* AbilitySystemComponent = Owner->FindComponentByClass<UAbilitySystemComponent>();
	if (!IsValid(AbilitySystemComponent)) return;
	
	const FGameplayAbilitySpecHandle AbilityHandle = ItemAbilityHandles[0];
	if (!AbilityHandle.IsValid()) return;
	
	FGameplayAbilitySpec* AbilitySpec = AbilitySystemComponent->FindAbilitySpecFromHandle(AbilityHandle);
	if (AbilitySpec == nullptr) return;
	
	AbilitySystemComponent->AbilitySpecInputPressed(*AbilitySpec);
	const UGameplayAbility* Ability = AbilitySpec ? AbilitySpec->Ability : nullptr;

	const bool bActivated = AbilitySystemComponent->TryActivateAbility(AbilityHandle);
}

void USGItemSlotComponent::UseItemReleased()
{
	if (ItemSlots.IsEmpty()) return;
	if (!ItemAbilityHandles.IsValidIndex(0)) return;
	
	// 해당 아이템이 유효한지, 실행할 Gameplay Ability가 있는지
	USGItemDefinition* ItemDefinition = ItemSlots[0];
	if (!IsValid(ItemDefinition) || !ItemDefinition->AbilityClass) return;
	
	AActor* Owner = GetOwner();
	if (!IsValid(Owner)) return;
	
	UAbilitySystemComponent* AbilitySystemComponent = Owner->FindComponentByClass<UAbilitySystemComponent>();
	if (!IsValid(AbilitySystemComponent)) return;
	
	const FGameplayAbilitySpecHandle AbilityHandle = ItemAbilityHandles[0];
	if (!AbilityHandle.IsValid()) return;
	
	FGameplayAbilitySpec* AbilitySpec = AbilitySystemComponent->FindAbilitySpecFromHandle(AbilityHandle);
	if (AbilitySpec == nullptr) return;

	const bool bWasActive = AbilitySpec->IsActive();
	
	UGameplayAbility* AbilityInstance = nullptr;
	FPredictionKey ActivationPredictionKey;
	
	if (bWasActive){
		AbilityInstance = AbilitySpec->GetPrimaryInstance();
		if (IsValid(AbilityInstance)){
			ActivationPredictionKey = AbilityInstance->GetCurrentActivationInfoRef().GetActivationPredictionKey();
		}
	}
	
	AbilitySystemComponent->AbilitySpecInputReleased(*AbilitySpec);
	
	AbilitySystemComponent->InvokeReplicatedEvent(
			EAbilityGenericReplicatedEvent::InputReleased,
			AbilityHandle,
			ActivationPredictionKey); 
	
	// 아이템 사용 성공 여부와는 관계없이 서버에서 소모처리
	if (!Owner->HasAuthority()){
		Server_ConsumeItem();
	}
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

void USGItemSlotComponent::Server_ConsumeItem_Implementation()
{
	if (ItemSlots.IsEmpty()) return;
	if (!ItemAbilityHandles.IsValidIndex(0)) return;
	
	ItemSlots.RemoveAt(0);
	ItemAbilityHandles.RemoveAt(0);
}
