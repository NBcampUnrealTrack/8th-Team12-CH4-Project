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
	virtual void HandleSeamlessTravelPlayer(AController*& Controller) override;
	
	// 플레이어 팀에 맞는 PlayerStart 선택
	virtual AActor* ChoosePlayerStart_Implementation(AController* Player) override;
	
	// 규칙 함수 
	void OnGoalScored(bool bIsRedTeamGoal);
	void EndMatch();
	void RestartRound();

protected:
	virtual void BeginPlay() override;

	// 초기 로딩 진입 및 프레임워크 검증 루프 
	void StartLoading();
	void UpdateLoadingProgress();
	
	// 게임 시작 및 월드 타이머 기동 
	void StartGame();
	void SpawnNewBall();
	void UpdateMatchTime();
	
protected:
	// 타이틀 룰 규칙
	const float UpdateLoadingTime = 0.1f; // 매직넘버 방지용
	const float UpdateStartTime = 5.0f;
	float LodingTime = 0.0f;
	
	UPROPERTY(EditDefaultsOnly, Category = "SG_Rules")
	int32 TotalMatchTime = 300;
	UPROPERTY(EditDefaultsOnly, Category = "SG_Rules")
	int32 ScoreToWin = 5;
	
	
	UPROPERTY(EditDefaultsOnly, Category = "SG_Spawning")
	TSubclassOf<AActor> BallClass;

	// --- 내부 관리용 변수 ---
	UPROPERTY(Transient)
	AActor* SpawnedBall;

	FTimerHandle MatchTimerHandle;
	FTimerHandle LoadingCheckTimerHandle;

	TMap<AController*, ASGPlayerStart*> AssignedInitialPlayerStarts;
	
	// 아직 사용하지않음
	/*
	UPROPERTY(Transient)
	TArray<AController*> RedTeamPlayers;
	UPROPERTY(Transient)
	TArray<AController*> BlueTeamPlayers;
	 */


};
