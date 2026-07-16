// Copyright Epic Games, Inc. All Rights Reserved.

#include "Audio/SGAudioSubsystem.h"

#include "Audio/SGAudioSettingsDataAsset.h"

#include "Components/AudioComponent.h"
#include "Engine/GameInstance.h"
#include "Engine/World.h"
#include "Kismet/GameplayStatics.h"
#include "Sound/SoundBase.h"
#include "UObject/ConstructorHelpers.h"
#include "UObject/UObjectGlobals.h"

DEFINE_LOG_CATEGORY_STATIC(LogSGAudio, Log, All);

namespace SGAudio
{
	const FName MainMenuMapName(TEXT("SG_MainMenu"));
	const FName LobbyMapName(TEXT("SG_LobbyLevel"));
	const FName GameplayMapName(TEXT("PlayBase"));
	const TCHAR* AudioSettingsPath = TEXT("/Game/SoccerGame/Audio/Settings/DataAsset/DA_AudioSettings.DA_AudioSettings");
	constexpr float BGMFadeInDuration = 1.0f;
	constexpr float BGMFadeOutDuration = 0.5f;
	constexpr float CrowdFadeInDuration = 2.0f;
	constexpr float CrowdFadeOutDuration = 0.5f;
}

USGAudioSubsystem::USGAudioSubsystem()
{
	static ConstructorHelpers::FObjectFinder<USoundBase> MainMenuBGMFinder(
		TEXT("/Game/SoccerGame/Dev/Audio/BGM/S_BGM_Main_TurfWar_01.S_BGM_Main_TurfWar_01"));
	MainMenuBGM = MainMenuBGMFinder.Object;

	static ConstructorHelpers::FObjectFinder<USoundBase> InGameBGMFinder(
		TEXT("/Game/SoccerGame/Dev/Audio/BGM/S_BGM_InGame_IndieRock_01.S_BGM_InGame_IndieRock_01"));
	InGameBGM = InGameBGMFinder.Object;

	static ConstructorHelpers::FObjectFinder<USoundBase> InGameCrowdFinder(
		TEXT("/Game/SoccerGame/Dev/Audio/InGame/S_InGame_CrowdLoop_01.S_InGame_CrowdLoop_01"));
	InGameCrowd = InGameCrowdFinder.Object;

	AudioSettingsAsset = TSoftObjectPtr<USGAudioSettingsDataAsset>(
		FSoftObjectPath(SGAudio::AudioSettingsPath));
}

bool USGAudioSubsystem::ShouldCreateSubsystem(UObject* Outer) const
{
	return Super::ShouldCreateSubsystem(Outer) && !IsRunningDedicatedServer();
}

void USGAudioSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

	PostLoadMapHandle = FCoreUObjectDelegates::PostLoadMapWithWorld.AddUObject(
		this, &ThisClass::HandlePostLoadMap);

	// 월드가 이미 로드된 뒤 핫 리로드되거나 서브시스템이 생성되는 경우도 처리합니다.
	RefreshAudioForCurrentMap();
}

void USGAudioSubsystem::Deinitialize()
{
	if (PostLoadMapHandle.IsValid())
	{
		FCoreUObjectDelegates::PostLoadMapWithWorld.Remove(PostLoadMapHandle);
		PostLoadMapHandle.Reset();
	}

	CurrentBGM = nullptr;
	CurrentCrowd = nullptr;
	StopAndReleaseComponent(BGMComponent);
	StopAndReleaseComponent(CrowdComponent);

	Super::Deinitialize();
}

void USGAudioSubsystem::PlayBGM(USoundBase* NewBGM, float FadeInDuration, float Volume)
{
	if (!IsValid(NewBGM))
	{
		UE_LOG(LogSGAudio, Warning, TEXT("PlayBGM ignored because the sound is invalid."));
		return;
	}

	CurrentBGMSceneVolume = FMath::Max(0.0f, Volume);

	if (IsValid(BGMComponent) && CurrentBGM == NewBGM && BGMComponent->IsPlaying())
	{
		BGMComponent->SetVolumeMultiplier(CurrentBGMSceneVolume * BGMVolumeMultiplier);
		return;
	}

	if (!IsValid(BGMComponent))
	{
		BGMComponent = CreatePersistent2DComponent(NewBGM);
		if (!IsValid(BGMComponent))
		{
			return;
		}

		BGMComponent->bIsUISound = true;
	}
	else
	{
		BGMComponent->Stop();
		BGMComponent->SetSound(NewBGM);
	}

	CurrentBGM = NewBGM;
	BGMComponent->FadeIn(
		FMath::Max(0.0f, FadeInDuration),
		CurrentBGMSceneVolume * BGMVolumeMultiplier,
		0.0f);
}

void USGAudioSubsystem::StopBGM(float FadeOutDuration)
{
	CurrentBGM = nullptr;

	if (!IsValid(BGMComponent) || !BGMComponent->IsPlaying())
	{
		return;
	}

	if (FadeOutDuration > 0.0f)
	{
		BGMComponent->FadeOut(FadeOutDuration, 0.0f);
	}
	else
	{
		BGMComponent->Stop();
	}
}

void USGAudioSubsystem::SetBGMVolume(float Volume)
{
	BGMVolumeMultiplier = FMath::Clamp(Volume, 0.0f, 1.0f);

	if (IsValid(BGMComponent))
	{
		BGMComponent->SetVolumeMultiplier(CurrentBGMSceneVolume * BGMVolumeMultiplier);
	}
}

void USGAudioSubsystem::PlayCrowd(USoundBase* NewCrowd, float FadeInDuration, float Volume)
{
	if (!IsValid(NewCrowd))
	{
		UE_LOG(LogSGAudio, Warning, TEXT("PlayCrowd ignored because the sound is invalid."));
		return;
	}

	CurrentCrowdSceneVolume = FMath::Max(0.0f, Volume);

	if (IsValid(CrowdComponent) && CurrentCrowd == NewCrowd && CrowdComponent->IsPlaying())
	{
		CrowdComponent->SetVolumeMultiplier(CurrentCrowdSceneVolume * CrowdVolumeMultiplier);
		return;
	}

	if (!IsValid(CrowdComponent))
	{
		CrowdComponent = CreatePersistent2DComponent(NewCrowd);
		if (!IsValid(CrowdComponent))
		{
			return;
		}

		// CreateSound2D는 기본적으로 UI 오디오로 생성되므로, SC_SFX인 관중 소리는 게임의 일시정지 규칙을 따르도록 설정합니다.
		CrowdComponent->bIsUISound = false;
	}
	else
	{
		CrowdComponent->Stop();
		CrowdComponent->SetSound(NewCrowd);
	}

	CurrentCrowd = NewCrowd;
	CrowdComponent->FadeIn(
		FMath::Max(0.0f, FadeInDuration),
		CurrentCrowdSceneVolume * CrowdVolumeMultiplier,
		0.0f);
}

void USGAudioSubsystem::StopCrowd(float FadeOutDuration)
{
	CurrentCrowd = nullptr;

	if (!IsValid(CrowdComponent) || !CrowdComponent->IsPlaying())
	{
		return;
	}

	if (FadeOutDuration > 0.0f)
	{
		CrowdComponent->FadeOut(FadeOutDuration, 0.0f);
	}
	else
	{
		CrowdComponent->Stop();
	}
}

void USGAudioSubsystem::SetCrowdVolume(float Volume)
{
	CrowdVolumeMultiplier = FMath::Clamp(Volume, 0.0f, 1.0f);

	if (IsValid(CrowdComponent))
	{
		CrowdComponent->SetVolumeMultiplier(CurrentCrowdSceneVolume * CrowdVolumeMultiplier);
	}
}

void USGAudioSubsystem::RefreshAudioForCurrentMap()
{
	// 에디터에서 설정 애셋을 새로 만든 뒤에도 이 API를 호출하면 다시 탐색할 수 있습니다.
	LoadAudioSettings();

	if (UGameInstance* GameInstance = GetGameInstance())
	{
		ApplyAudioForMap(GameInstance->GetWorld());
	}
}

void USGAudioSubsystem::HandlePostLoadMap(UWorld* LoadedWorld)
{
	// 에디터에서 설정 애셋을 생성하거나 수정한 뒤 다음 맵으로 이동하면 최신 값을 다시 적용합니다.
	LoadAudioSettings();
	ApplyAudioForMap(LoadedWorld);
}

void USGAudioSubsystem::ApplyAudioForMap(UWorld* World)
{
	if (!IsAudioWorld(World))
	{
		return;
	}

	const FName MapName(*UGameplayStatics::GetCurrentLevelName(World, true));

	const float MainMenuBGMVolume = IsValid(AudioSettings) ? AudioSettings->MainMenuBGMVolume : 0.7f;
	const float InGameBGMVolume = IsValid(AudioSettings) ? AudioSettings->InGameBGMVolume : 0.4f;
	const float InGameCrowdVolume = IsValid(AudioSettings) ? AudioSettings->InGameCrowdVolume : 0.3f;

	if (MapName == SGAudio::MainMenuMapName || MapName == SGAudio::LobbyMapName)
	{
		PlayBGM(MainMenuBGM, SGAudio::BGMFadeInDuration, MainMenuBGMVolume);
		StopCrowd(SGAudio::CrowdFadeOutDuration);
		UE_LOG(LogSGAudio, Log, TEXT("Applied menu audio scene for map %s."), *MapName.ToString());
		return;
	}

	if (MapName == SGAudio::GameplayMapName)
	{
		PlayBGM(InGameBGM, SGAudio::BGMFadeInDuration, InGameBGMVolume);
		PlayCrowd(InGameCrowd, SGAudio::CrowdFadeInDuration, InGameCrowdVolume);
		UE_LOG(LogSGAudio, Log, TEXT("Applied gameplay audio scene for map %s."), *MapName.ToString());
		return;
	}

	// 등록되지 않은 테스트 맵에서는 유지형 오디오가 계속 재생되지 않도록 정지합니다.
	StopBGM(SGAudio::BGMFadeOutDuration);
	StopCrowd(SGAudio::CrowdFadeOutDuration);
	UE_LOG(LogSGAudio, Verbose, TEXT("Stopped global audio for unregistered map %s."), *MapName.ToString());
}

void USGAudioSubsystem::LoadAudioSettings()
{
	USGAudioSettingsDataAsset* LoadedSettings = AudioSettingsAsset.LoadSynchronous();

	if (!IsValid(LoadedSettings))
	{
		AudioSettings = nullptr;
		UE_LOG(LogSGAudio, Verbose, TEXT("DA_AudioSettings가 없어 C++ 기본 오디오 설정을 사용합니다."));
		return;
	}

	AudioSettings = LoadedSettings;

	// 비어 있는 항목은 기존 C++ 하드 레퍼런스를 유지해 설정 실수로 재생이 끊기지 않게 합니다.
	if (IsValid(AudioSettings->MainMenuBGM))
	{
		MainMenuBGM = AudioSettings->MainMenuBGM;
	}
	if (IsValid(AudioSettings->InGameBGM))
	{
		InGameBGM = AudioSettings->InGameBGM;
	}
	if (IsValid(AudioSettings->InGameCrowd))
	{
		InGameCrowd = AudioSettings->InGameCrowd;
	}

	UE_LOG(LogSGAudio, Log, TEXT("DA_AudioSettings를 적용했습니다."));
}

bool USGAudioSubsystem::IsAudioWorld(const UWorld* World) const
{
	return IsValid(World)
		&& World->IsGameWorld()
		&& World->GetGameInstance() == GetGameInstance()
		&& World->GetNetMode() != NM_DedicatedServer;
}

UAudioComponent* USGAudioSubsystem::CreatePersistent2DComponent(USoundBase* Sound) const
{
	if (!IsValid(Sound))
	{
		return nullptr;
	}

	return UGameplayStatics::CreateSound2D(
		GetGameInstance(),
		Sound,
		1.0f,
		1.0f,
		0.0f,
		nullptr,
		true,
		false);
}

void USGAudioSubsystem::StopAndReleaseComponent(TObjectPtr<UAudioComponent>& Component)
{
	if (!IsValid(Component))
	{
		Component = nullptr;
		return;
	}

	Component->Stop();
	Component->DestroyComponent();
	Component = nullptr;
}
