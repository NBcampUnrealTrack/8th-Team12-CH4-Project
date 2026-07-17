// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "SGAudioSettingsDataAsset.generated.h"

class USoundBase;

/**
 * 맵별 BGM과 관중 루프의 애셋과 기본 볼륨을 에디터에서 관리합니다.
 * 런타임 사용자 볼륨은 SGAudioSubsystem의 SetBGMVolume/SetCrowdVolume에서 별도로 적용합니다.
 */
UCLASS(BlueprintType)
class SOCCERGAME_API USGAudioSettingsDataAsset final : public UDataAsset
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Audio|Sounds")
	TObjectPtr<USoundBase> MainMenuBGM;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Audio|Sounds")
	TObjectPtr<USoundBase> InGameBGM;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Audio|Sounds")
	TObjectPtr<USoundBase> InGameCrowd;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Audio|Volume", meta = (ClampMin = "0.0", ClampMax = "2.0", UIMin = "0.0", UIMax = "2.0"))
	float MainMenuBGMVolume = 0.7f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Audio|Volume", meta = (ClampMin = "0.0", ClampMax = "2.0", UIMin = "0.0", UIMax = "2.0"))
	float InGameBGMVolume = 0.4f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Audio|Volume", meta = (ClampMin = "0.0", ClampMax = "2.0", UIMin = "0.0", UIMax = "2.0"))
	float InGameCrowdVolume = 0.3f;
};
