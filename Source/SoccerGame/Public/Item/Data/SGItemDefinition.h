// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "Engine/DataAsset.h"
#include "SGItemDefinition.generated.h"

class UGameplayEffect;
class UGameplayAbility;
/**
 * 
 */
UCLASS()
class SOCCERGAME_API USGItemDefinition : public UPrimaryDataAsset
{
	GENERATED_BODY()
	
public:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Item")
	TObjectPtr<UTexture2D> Icon;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Item")
	FGameplayTag ItemTag;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Item")
	TSubclassOf<UGameplayAbility> AbilityClass;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Item")
	TSubclassOf<UGameplayEffect> EffectClass;
};
