// Copyright Epic Games, Inc. All Rights Reserved.

#include "Audio/SGInGameAudioSubsystem.h"

#include "Audio/SGInGameAudioSettingsDataAsset.h"
#include "Engine/World.h"
#include "Kismet/GameplayStatics.h"
#include "Sound/SoundBase.h"

DEFINE_LOG_CATEGORY_STATIC(LogSGInGameAudio, Log, All);

namespace SGInGameAudio
{
	const TCHAR* AudioSettingsPath = TEXT("/Game/SoccerGame/Audio/Settings/DataAsset/DA_InGameAudioSettings.DA_InGameAudioSettings");
}

USGInGameAudioSubsystem::USGInGameAudioSubsystem()
{
	AudioSettingsAsset = TSoftObjectPtr<USGInGameAudioSettingsDataAsset>(
		FSoftObjectPath(SGInGameAudio::AudioSettingsPath));
}

bool USGInGameAudioSubsystem::ShouldCreateSubsystem(UObject* Outer) const
{
	return Super::ShouldCreateSubsystem(Outer) && !IsRunningDedicatedServer();
}

void USGInGameAudioSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);
	ReloadAudioSettings();
}

void USGInGameAudioSubsystem::HandleAudioEvent(ESGInGameAudioEvent AudioEvent)
{
	switch (AudioEvent)
	{
	case ESGInGameAudioEvent::GoalCelebration:
		PlayGoalCelebration();
		break;
	case ESGInGameAudioEvent::MatchStartWhistle:
		PlayMatchStartWhistle();
		break;
	case ESGInGameAudioEvent::MatchResumeWhistle:
		PlayMatchResumeWhistle();
		break;
	case ESGInGameAudioEvent::MatchEndWhistle:
		PlayMatchEndWhistle();
		break;
	case ESGInGameAudioEvent::MatchEndWarning:
		PlayMatchEndWarning();
		break;
	case ESGInGameAudioEvent::CountdownThree:
		PlayCountdown(3);
		break;
	case ESGInGameAudioEvent::CountdownTwo:
		PlayCountdown(2);
		break;
	case ESGInGameAudioEvent::CountdownOne:
		PlayCountdown(1);
		break;
	case ESGInGameAudioEvent::CountdownFinal:
		PlayCountdownFinal();
		break;
	default:
		UE_LOG(LogSGInGameAudio, Warning, TEXT("알 수 없는 인게임 오디오 이벤트를 무시합니다."));
		break;
	}
}

void USGInGameAudioSubsystem::ReloadAudioSettings()
{
	AudioSettings = AudioSettingsAsset.LoadSynchronous();
	if (!IsValid(AudioSettings))
	{
		UE_LOG(LogSGInGameAudio, Warning, TEXT("DA_InGameAudioSettings를 찾지 못했습니다: %s"), SGInGameAudio::AudioSettingsPath);
	}
}

void USGInGameAudioSubsystem::PlayGoalCelebration()
{
	if (!EnsureAudioSettings())
	{
		return;
	}

	// 기본 관중 루프를 끊지 않고 골 함성과 불꽃을 전역 원샷 레이어로 함께 재생합니다.
	PlayGlobal2D(AudioSettings->GoalCheer, AudioSettings->GoalCheerVolume);
	PlayGlobal2D(AudioSettings->GoalFireworks, AudioSettings->GoalFireworksVolume);
}

void USGInGameAudioSubsystem::PlayMatchStartWhistle()
{
	if (EnsureAudioSettings()) PlayGlobal2D(AudioSettings->MatchStartWhistle, AudioSettings->WhistleVolume);
}

void USGInGameAudioSubsystem::PlayMatchResumeWhistle()
{
	if (EnsureAudioSettings()) PlayGlobal2D(AudioSettings->MatchResumeWhistle, AudioSettings->WhistleVolume);
}

void USGInGameAudioSubsystem::PlayMatchEndWhistle()
{
	if (EnsureAudioSettings()) PlayGlobal2D(AudioSettings->MatchEndWhistle, AudioSettings->WhistleVolume);
}

void USGInGameAudioSubsystem::PlayMatchEndWarning()
{
	if (EnsureAudioSettings()) PlayGlobal2D(AudioSettings->MatchEndWarning, AudioSettings->MatchEndWarningVolume);
}

void USGInGameAudioSubsystem::PlayCountdown(int32 Seconds)
{
	if (!EnsureAudioSettings())
	{
		return;
	}

	USoundBase* CountdownSound = nullptr;
	switch (Seconds)
	{
	case 3:
		CountdownSound = AudioSettings->CountdownBeep01;
		break;
	case 2:
		CountdownSound = AudioSettings->CountdownBeep02;
		break;
	case 1:
		CountdownSound = AudioSettings->CountdownBeep03;
		break;
	default:
		UE_LOG(LogSGInGameAudio, Verbose, TEXT("지원하지 않는 카운트다운 값입니다: %d"), Seconds);
		return;
	}

	PlayGlobal2D(CountdownSound, AudioSettings->CountdownVolume);
}

void USGInGameAudioSubsystem::PlayCountdownFinal()
{
	if (EnsureAudioSettings()) PlayGlobal2D(AudioSettings->CountdownFinal, AudioSettings->CountdownVolume);
}

void USGInGameAudioSubsystem::PlayGlobal2D(USoundBase* Sound, float Volume) const
{
	UWorld* World = GetWorld();
	if (!IsValid(World) || !World->IsGameWorld() || World->GetNetMode() == NM_DedicatedServer)
	{
		return;
	}

	if (!IsValid(Sound))
	{
		UE_LOG(LogSGInGameAudio, Warning, TEXT("DA_InGameAudioSettings의 비어 있는 사운드 항목을 재생하지 않습니다."));
		return;
	}

	UGameplayStatics::PlaySound2D(
		World,
		Sound,
		FMath::Clamp(Volume, 0.0f, 2.0f),
		1.0f,
		0.0f,
		nullptr,
		nullptr,
		false);
}

bool USGInGameAudioSubsystem::EnsureAudioSettings()
{
	if (!IsValid(AudioSettings))
	{
		ReloadAudioSettings();
	}

	return IsValid(AudioSettings);
}
