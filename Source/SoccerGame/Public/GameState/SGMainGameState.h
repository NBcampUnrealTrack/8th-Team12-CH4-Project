// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameStateBase.h"
#include "GameplayTagContainer.h"
#include "SGMainGameState.generated.h"
// 경기 상태 관리를 위한 열거형 (GameState 헤더로 이동)
UENUM(BlueprintType)
enum class ESGMatchState : uint8
{
	WaitingToStart, // 경기 시작 전 대기 (카운트다운 등)
	InProgress,     // 경기 진행 중
	GoalScored,     // 골 발생 (잠시 멈춤 및 연출)
	GameOver        // 경기 완전히 종료
};

class USGInGameWidget;
UCLASS()
class SOCCERGAME_API ASGMainGameState : public AGameStateBase
{
	GENERATED_BODY()
	
public:
	ASGMainGameState();
	// 네트워크 복제를 위한 변수 등록 함수
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
public:
    
	// 현재 경기 상태 (값이 바뀌면 OnRep_MatchState 호출)
	UPROPERTY(ReplicatedUsing = OnRep_MatchState, BlueprintReadOnly, Category = "SG_State")
	FGameplayTag CurrentMatchStateTag;

	UPROPERTY(ReplicatedUsing = OnRep_UpdateScore, BlueprintReadOnly, Category = "SG_Score")
	int32 RedTeamScore =0 ;

	UPROPERTY(ReplicatedUsing = OnRep_UpdateScore, BlueprintReadOnly, Category = "SG_Score")
	int32 BlueTeamScore =0;

	UPROPERTY(ReplicatedUsing = OnRep_UpdateTime, BlueprintReadOnly, Category = "SG_Time")
	int32 CurrentGameTime = 0;
public:
	// --- OnRep 함수들 (클라이언트 UI 갱신용 노티파이) ---

	UFUNCTION()
	void OnRep_MatchState();
	UFUNCTION()
	void OnRep_UpdateScore();
	UFUNCTION()
	void OnRep_UpdateTime();

	
};
