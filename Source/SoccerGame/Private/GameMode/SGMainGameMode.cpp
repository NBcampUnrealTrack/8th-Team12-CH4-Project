// Fill out your copyright notice in the Description page of Project Settings.


#include "GameMode/SGMainGameMode.h"
#include "SoccerGame/Public/GameState/SGMainGameState.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/PlayerStart.h"
#include "SoccerGame/Public/GameState/SGMainGameState.h"

ASGMainGameMode::ASGMainGameMode()
{
	// GameMode는 오직 서버에만 존재하므로 복제(Replicate)할 필요가 없습니다.
	bReplicates = false;
	UE_LOG(LogTemp, Warning, TEXT("In Main Server"));
	
}

void ASGMainGameMode::BeginPlay()
{
	Super::BeginPlay();
	
	// PlayerController 보다 먼저 호출됨
	
	UE_LOG(LogTemp, Warning, TEXT("In Main Server"));
	
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

void ASGMainGameMode::StartLoading()
{
	// 시작 전 초기화
	
	 // 여기서 스폰을 할까?
}

void ASGMainGameMode::StartGame()
{
	// 게임 시작 
	// UpdateMatchTime 
	
}


void ASGMainGameMode::OnGoalScored(bool bIsRedTeamGoal)
{

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