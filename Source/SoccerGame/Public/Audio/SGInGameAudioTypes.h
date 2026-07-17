// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "SGInGameAudioTypes.generated.h"

/** 서버가 각 화면의 로컬 인게임 오디오 시스템에 전달할 전역 경기 이벤트입니다. */
UENUM(BlueprintType)
enum class ESGInGameAudioEvent : uint8
{
	GoalCelebration,
	MatchStartWhistle,
	MatchResumeWhistle,
	MatchEndWhistle,
	MatchEndWarning,
	CountdownThree,
	CountdownTwo,
	CountdownOne,
	CountdownFinal
};
