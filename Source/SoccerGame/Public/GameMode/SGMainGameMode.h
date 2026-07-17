// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "GameFramework/Character.h"
#include "GameplayTagContainer.h"
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
	virtual UClass* GetDefaultPawnClassForController_Implementation(AController* InController) override;
	void OnGoalScored(FGameplayTag GoalTeamTag);
	
	//UPROPERTY(EditDefaultsOnly, Category = "SG_Rules")
	//FString ResultLevelPath = TEXT("/Game/SoccerGame/Maps/");
	
	// 세레머니 타임 
	UPROPERTY(EditDefaultsOnly, Category = "SG_Rules")
	float TransitionToResultDelay = 5.0f;
	FTimerHandle ResultTransitionTimerHandle;
	
	// 규칙 함수 
protected:
	virtual void BeginPlay() override;

	// 초기 로딩 진입 및 프레임워크 검증 루프 
	void StartLoading();
	void UpdateLoadingProgress();
	
	// 게임 시작 및 월드 타이머 기동 
	void StartGame();
	void UpdateMatchTime();
	
	void SpawnNewBall();
	
	void EndMatch();
	bool EndScoreMatch();
	void WinTeamCheck();
	// 잠시 보류  게임 종료해서 결과 Level 로 넘어가는 걸로 수정 
	void RestartRound();
	
	//// 플레이어 데이터 저장 이동 시키는 함수 
	//void StartResultTransition();

	// Level 이동 함수 
	void TransitionToResultLevel();

protected:
	// 타이틀 룰 규칙
	const float UpdateLoadingTime = 0.1f; // 매직넘버 방지용
	const float UpdateStartTime = 5.0f;
	float CurrentLoadingTime = 0.0f;
	
	UPROPERTY(EditDefaultsOnly, Category = "SG_Rules")
	int32 TotalMatchTime = 300;
	UPROPERTY(EditDefaultsOnly, Category = "SG_Rules")
	int32 ScoreToWin = 5;
	
	UPROPERTY(EditDefaultsOnly, Category = "SG_Spawning")
	TSubclassOf<AActor> BallClass;
	
	UPROPERTY(EditDefaultsOnly, Category = "SG_Spawning")
	FGameplayTag WinTeamTag = FGameplayTag::RequestGameplayTag(FName("Match.Result.Draw"));

	UPROPERTY(EditDefaultsOnly, Category = "SG_Spawning|Character")
	TMap<FGameplayTag, TSubclassOf<ACharacter>> CharacterCatalog;

	// --- 내부 관리용 변수 ---
	UPROPERTY(Transient)
	AActor* SpawnedBall;

	FTimerHandle MatchTimerHandle;
	// 로딩 진행 상황을 체크할 타이머 핸들
	FTimerHandle LoadingCheckTimerHandle;
	FTimerHandle RoundRestartTimerHandle;
	
	TMap<AController*, ASGPlayerStart*> AssignedInitialPlayerStarts;
	
	// 골 이후, 연출 등을 위해 재시작 이전 딜레이
	UPROPERTY(EditDefaultsOnly, Category = "SG_Rules")
	float GoalRestartDelay = 3.0f;

	// 공 스폰 위치를 찾기 위한 Tag
	UPROPERTY(EditDefaultsOnly, Category = "SG_Spawning")
	FName BallSpawnTag = TEXT("BallSpawn");
	
	// 아직 사용하지않음
	/*
	UPROPERTY(Transient)
	TArray<AController*> RedTeamPlayers;
	UPROPERTY(Transient)
	TArray<AController*> BlueTeamPlayers;
	 */


};
