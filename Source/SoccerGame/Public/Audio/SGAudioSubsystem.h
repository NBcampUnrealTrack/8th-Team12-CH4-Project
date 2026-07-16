// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "SGAudioSubsystem.generated.h"

class UAudioComponent;
class USGAudioSettingsDataAsset;
class USoundBase;

/**
 * 하나의 게임 인스턴스에서 맵 전환에도 유지되는 비공간화 BGM과 관중 소리를 관리합니다.
 *
 * 오디오는 의도적으로 로컬에서만 재생합니다. 리슨 서버 호스트와 원격 클라이언트는
 * 각각 자신의 게임 인스턴스를 가지므로 오디오 컴포넌트를 복제하지 않고 각자 재생합니다.
 */
UCLASS()
class SOCCERGAME_API USGAudioSubsystem final : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	USGAudioSubsystem();

	virtual bool ShouldCreateSubsystem(UObject* Outer) const override;
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;

	UFUNCTION(BlueprintCallable, Category = "SG|Audio")
	void PlayBGM(USoundBase* NewBGM, float FadeInDuration = 1.0f, float Volume = 1.0f);

	UFUNCTION(BlueprintCallable, Category = "SG|Audio")
	void StopBGM(float FadeOutDuration = 1.0f);

	/**
	 * 현재 및 이후에 재생할 BGM에 0.0~1.0 범위의 추가 볼륨 배율을 적용합니다.
	 * 주의: 최종 볼륨은 장면별 기본 볼륨(메뉴 0.7, 인게임 0.4), 이 값, SC_BGM 볼륨을 모두 곱한 결과입니다.
	 * 이 값은 게임 인스턴스가 유지되는 동안만 보존되며 디스크에 자동 저장되지 않습니다.
	 */
	UFUNCTION(BlueprintCallable, Category = "SG|Audio")
	void SetBGMVolume(float Volume);

	UFUNCTION(BlueprintCallable, Category = "SG|Audio")
	void PlayCrowd(USoundBase* NewCrowd, float FadeInDuration = 1.0f, float Volume = 1.0f);

	UFUNCTION(BlueprintCallable, Category = "SG|Audio")
	void StopCrowd(float FadeOutDuration = 1.0f);

	/**
	 * 현재 및 이후에 재생할 관중 소리에 0.0~1.0 범위의 추가 볼륨 배율을 적용합니다.
	 * 주의: 최종 볼륨은 장면별 기본 볼륨(0.3), 이 값, SC_SFX 볼륨을 모두 곱한 결과입니다.
	 * SC_SFX를 공유하는 다른 효과음의 볼륨은 변경하지 않으며 디스크에도 자동 저장되지 않습니다.
	 */
	UFUNCTION(BlueprintCallable, Category = "SG|Audio")
	void SetCrowdVolume(float Volume);

	/** 게임 인스턴스의 현재 맵에 맞는 오디오 구성을 다시 적용합니다. */
	UFUNCTION(BlueprintCallable, Category = "SG|Audio")
	void RefreshAudioForCurrentMap();

private:
	void LoadAudioSettings();
	void HandlePostLoadMap(UWorld* LoadedWorld);
	void ApplyAudioForMap(UWorld* World);
	bool IsAudioWorld(const UWorld* World) const;

	UAudioComponent* CreatePersistent2DComponent(USoundBase* Sound) const;
	static void StopAndReleaseComponent(TObjectPtr<UAudioComponent>& Component);

private:
	/** Content/SoccerGame/Audio/Settings/DA_AudioSettings에서 불러온 맵 오디오 설정입니다. */
	UPROPERTY()
	TObjectPtr<USGAudioSettingsDataAsset> AudioSettings;

	/** 문자열 직접 로드가 아니라 소프트 레퍼런스로 설정 애셋의 고정 경로를 추적합니다. */
	UPROPERTY()
	TSoftObjectPtr<USGAudioSettingsDataAsset> AudioSettingsAsset;

	/** 수동 공유되는 오디오 애셋을 쿠커가 확실히 찾을 수 있도록 하드 레퍼런스로 유지합니다. */
	UPROPERTY()
	TObjectPtr<USoundBase> MainMenuBGM;

	UPROPERTY()
	TObjectPtr<USoundBase> InGameBGM;

	UPROPERTY()
	TObjectPtr<USoundBase> InGameCrowd;

	UPROPERTY(Transient)
	TObjectPtr<UAudioComponent> BGMComponent;

	UPROPERTY(Transient)
	TObjectPtr<UAudioComponent> CrowdComponent;

	UPROPERTY(Transient)
	TObjectPtr<USoundBase> CurrentBGM;

	UPROPERTY(Transient)
	TObjectPtr<USoundBase> CurrentCrowd;

	UPROPERTY(Transient)
	float BGMVolumeMultiplier = 1.0f;

	UPROPERTY(Transient)
	float CrowdVolumeMultiplier = 1.0f;

	UPROPERTY(Transient)
	float CurrentBGMSceneVolume = 1.0f;

	UPROPERTY(Transient)
	float CurrentCrowdSceneVolume = 1.0f;

	FDelegateHandle PostLoadMapHandle;
};
