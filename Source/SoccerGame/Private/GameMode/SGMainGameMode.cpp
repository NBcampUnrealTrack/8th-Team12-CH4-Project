// Fill out your copyright notice in the Description page of Project Settings.


#include "GameMode/SGMainGameMode.h"
#include "SoccerGame/Public/GameState/SGMainGameState.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/PlayerStart.h"

ASGMainGameMode::ASGMainGameMode()
{
	// GameMode는 오직 서버에만 존재하므로 복제(Replicate)할 필요가 없습니다.
	bReplicates = false;
}

void ASGMainGameMode::BeginPlay()
{
	Super::BeginPlay();
}


void ASGMainGameMode::OnGoalScored(bool bIsRedTeamGoal)
{
	// 1. 플레이어 팀 분배 (접속 유저 기준 3:3 분배)
	int32 PlayerIndex = 0;
	for (FConstPlayerControllerIterator It = GetWorld()->GetPlayerControllerIterator(); It; ++It)
	{
		APlayerController* PC = It->Get();
		if (PC)
		{
			if (PlayerIndex % 2 == 0) RedTeamPlayers.Add(PC);
			else BlueTeamPlayers.Add(PC);
			PlayerIndex++;
		}
	}
	// 2. 초기 맵 세팅 및 공 스폰
	MovePlayersToSpawnPoints();
	SpawnNewBall();

	// 3. GameState(전광판) 초기값 세팅 및 시작 명령
	ASGMainGameState* SG_GameState = GetGameState<ASGMainGameState>();
	if (SG_GameState)
	{
		SG_GameState->CurrentGameTime = TotalMatchTime;
		SG_GameState->RedTeamScore = 0;
		SG_GameState->BlueTeamScore = 0;
		// 직접 들고 있지 않고, GameState의 상태 변수를 변경하여 클라이언트에 전파!
		SG_GameState->CurrentMatchState = ESGMatchState::InProgress;
	}

	// 4. 1초 간격 인게임 타이머 가동
	GetWorldTimerManager().SetTimer(MatchTimerHandle, this, &ASGMainGameMode::UpdateMatchTime, 1.0f, true);
}

void ASGMainGameMode::EndMatch()
{
}

void ASGMainGameMode::RestartRound()
{
}

void ASGMainGameMode::SpawnNewBall()
{
}

void ASGMainGameMode::UpdateMatchTime()
{
}

void ASGMainGameMode::MovePlayersToSpawnPoints()
{
}