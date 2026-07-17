// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "SGInGameAudioSettingsDataAsset.generated.h"

class USoundBase;

/**
 * 한 경기 안에서 발생하는 전역 원샷 사운드와 기본 볼륨을 관리합니다.
 * 사운드웨이브를 코드에 직접 고정하지 않고 DA_InGameAudioSettings에서 할당합니다.
 */
UCLASS(BlueprintType)
class SOCCERGAME_API USGInGameAudioSettingsDataAsset final : public UDataAsset
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Audio|Goal")
	TObjectPtr<USoundBase> GoalCheer;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Audio|Goal")
	TObjectPtr<USoundBase> GoalFireworks;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Audio|Whistle")
	TObjectPtr<USoundBase> MatchStartWhistle;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Audio|Whistle")
	TObjectPtr<USoundBase> MatchResumeWhistle;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Audio|Whistle")
	TObjectPtr<USoundBase> MatchEndWhistle;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Audio|Match")
	TObjectPtr<USoundBase> MatchEndWarning;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Audio|Countdown")
	TObjectPtr<USoundBase> CountdownBeep01;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Audio|Countdown")
	TObjectPtr<USoundBase> CountdownBeep02;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Audio|Countdown")
	TObjectPtr<USoundBase> CountdownBeep03;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Audio|Countdown")
	TObjectPtr<USoundBase> CountdownFinal;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Audio|Volume", meta = (ClampMin = "0.0", ClampMax = "2.0", UIMin = "0.0", UIMax = "2.0"))
	float GoalCheerVolume = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Audio|Volume", meta = (ClampMin = "0.0", ClampMax = "2.0", UIMin = "0.0", UIMax = "2.0"))
	float GoalFireworksVolume = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Audio|Volume", meta = (ClampMin = "0.0", ClampMax = "2.0", UIMin = "0.0", UIMax = "2.0"))
	float WhistleVolume = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Audio|Volume", meta = (ClampMin = "0.0", ClampMax = "2.0", UIMin = "0.0", UIMax = "2.0"))
	float MatchEndWarningVolume = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Audio|Volume", meta = (ClampMin = "0.0", ClampMax = "2.0", UIMin = "0.0", UIMax = "2.0"))
	float CountdownVolume = 1.0f;
};
