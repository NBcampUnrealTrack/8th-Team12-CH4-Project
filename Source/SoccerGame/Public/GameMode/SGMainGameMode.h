// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "SGMainGameMode.generated.h"

class ASGPlayerStart;

UCLASS()
class SOCCERGAME_API ASGMainGameMode : public AGameModeBase
{
	GENERATED_BODY()
public:
	ASGMainGameMode();
	
	//Seamless Travel 모드에서 PlayerController Input Mode 전환
	virtual void HandleSeamlessTravelPlayer(AController*& C) override;

	// 플레이어 팀에 맞는 PlayerStart 선택
	virtual AActor* ChoosePlayerStart_Implementation(AController* Player) override;

	// 골이 발생했을 때 골대 트리거에서 호출할 심판 함수 (서버 전용)
	void OnGoalScored(bool bIsRedTeamGoal);

	// 경기를 완전히 끝내는 함수
	void EndMatch();

	// 골 연출이 끝난 후 다음 라운드를 시작하는 함수
	void RestartRound();

protected:
	virtual void BeginPlay() override;

	// 시작전 로딩 + 초기화 
	void StartLoading();
	
	// 게임 시작 
	void StartGame();

	// 축구공을 맵 중앙에 스폰하는 함수
	void SpawnNewBall();

	// 1초마다 타이머를 줄이는 함수
	void UpdateMatchTime();

protected:
	// --- 에디터 설정 규칙 변수 ---
	UPROPERTY(EditDefaultsOnly, Category = "SG_Rules")
	float TotalMatchTime = 300.0f;

	UPROPERTY(EditDefaultsOnly, Category = "SG_Rules")
	int32 ScoreToWin = 5;

	UPROPERTY(EditDefaultsOnly, Category = "SG_Spawning")
	TSubclassOf<AActor> BallClass;

	// --- 내부 관리용 변수 ---
	UPROPERTY(Transient)
	AActor* SpawnedBall;

	UPROPERTY(Transient)
	TArray<AController*> RedTeamPlayers;

	UPROPERTY(Transient)
	TArray<AController*> BlueTeamPlayers;

	FTimerHandle MatchTimerHandle;
	FTimerHandle RoundRestartTimerHandle;
	FTimerHandle LoadingCheckTimerHandle;

	TMap<AController*, ASGPlayerStart*> AssignedInitialPlayerStarts;

};
