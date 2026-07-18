// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "Engine/DataAsset.h"
#include "SGCharacterDataAsset.generated.h"

/**
 * 
 */
UCLASS()
class SOCCERGAME_API USGCharacterDataAsset : public UPrimaryDataAsset
{
	GENERATED_BODY()
	
public:
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FGameplayTag CharacterTag;
	
	UPROPERTY(EditAnywhere, BLueprintReadOnly)
	TSubclassOf<ACharacter> Character;
	
	UPROPERTY(EditAnywhere, BLueprintReadOnly)
	UTexture2D* Thumbnail;
};
