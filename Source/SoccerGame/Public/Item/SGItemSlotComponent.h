// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "SGItemSlotComponent.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnItemSlotChanged);

class USGItemDefinition;

UCLASS( ClassGroup=(Item), meta=(BlueprintSpawnableComponent) )
class SOCCERGAME_API USGItemSlotComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	USGItemSlotComponent();
	
	virtual void GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const override;

public:	
	// 아이템 획득
	bool AddItem(USGItemDefinition* NewItem);
	
	// 아이템 소모
	bool ConsumeItem();

public:
	// Getter
	UFUNCTION(BlueprintPure, Category = "Item|Inventory")
	int32 GetItemCount() const;
	
	UFUNCTION(BlueprintPure, Category = "Item|Inventory")
	USGItemDefinition* GetItemAt(int32 Index) const;

private:
    UFUNCTION()
    void OnRep_ItemSlots();
	
public:
	// Delegate
	// Item Slot 갱신
	UPROPERTY(BlueprintAssignable, Category = "Item|Inventory")
	FOnItemSlotChanged OnItemSlotChanged;
	
private:
	int32 MaxItemCount;
	
	UPROPERTY(ReplicatedUsing = OnRep_ItemSlots)
	TArray<TObjectPtr<USGItemDefinition>> ItemSlots;
};
