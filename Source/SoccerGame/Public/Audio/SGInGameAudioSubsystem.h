// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "Audio/SGInGameAudioTypes.h"
#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "SGInGameAudioSubsystem.generated.h"

class USGInGameAudioSettingsDataAsset;
class USoundBase;

/**
 * 현재 경기 월드에서 발생하는 전역 원샷 사운드를 로컬로 재생합니다.
 * 서버는 사운드 애셋이 아닌 ESGInGameAudioEvent만 전달합니다.
 */
UCLASS()
class SOCCERGAME_API USGInGameAudioSubsystem final : public UWorldSubsystem
{
	GENERATED_BODY()

public:
	USGInGameAudioSubsystem();

	virtual bool ShouldCreateSubsystem(UObject* Outer) const override;
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;

	/** GameState Multicast로 전달받은 이벤트를 로컬 재생 함수로 분기합니다. */
	void HandleAudioEvent(ESGInGameAudioEvent AudioEvent);

	UFUNCTION(BlueprintCallable, Category = "SG|Audio|InGame")
	void ReloadAudioSettings();

	UFUNCTION(BlueprintCallable, Category = "SG|Audio|InGame")
	void PlayGoalCelebration();

	UFUNCTION(BlueprintCallable, Category = "SG|Audio|InGame")
	void PlayMatchStartWhistle();

	UFUNCTION(BlueprintCallable, Category = "SG|Audio|InGame")
	void PlayMatchResumeWhistle();

	UFUNCTION(BlueprintCallable, Category = "SG|Audio|InGame")
	void PlayMatchEndWhistle();

	UFUNCTION(BlueprintCallable, Category = "SG|Audio|InGame")
	void PlayMatchEndWarning();

	UFUNCTION(BlueprintCallable, Category = "SG|Audio|InGame")
	void PlayCountdown(int32 Seconds);

	UFUNCTION(BlueprintCallable, Category = "SG|Audio|InGame")
	void PlayCountdownFinal();

private:
	void PlayGlobal2D(USoundBase* Sound, float Volume) const;
	bool EnsureAudioSettings();

private:
	/** 고정 경로의 DA_InGameAudioSettings를 패키징과 런타임 로드 대상으로 추적합니다. */
	UPROPERTY()
	TSoftObjectPtr<USGInGameAudioSettingsDataAsset> AudioSettingsAsset;

	UPROPERTY()
	TObjectPtr<USGInGameAudioSettingsDataAsset> AudioSettings;
};
