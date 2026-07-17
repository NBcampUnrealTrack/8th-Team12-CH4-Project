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
class SOCCERGAME_API USGCharacterDataAsset : public UDataAsset
{
	GENERATED_BODY()
	
public:
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FGameplayTag CharacterTag;
	
	UPROPERTY(EditAnywhere, BLueprintReadOnly)
	ACharacter* Character;
	
	UPROPERTY()
	UTexture2D* Thumbnail;
};
