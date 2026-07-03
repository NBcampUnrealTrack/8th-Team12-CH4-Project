// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameStateBase.h"
#include "SGLobbyGameState.generated.h"

/**
 * 
 */
UCLASS()
class SOCCERGAME_API ASGLobbyGameState : public AGameStateBase
{
	GENERATED_BODY()
public:
	ASGLobbyGameState();
	// 멀티플레이어 변수 동기화를 위한 설정 오버라이드
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	// 플레이어의 준비 상태 (서버가 수정하면 전체 클라이언트에 복제)
	UPROPERTY(ReplicatedUsing = OnRep_IsReady, BlueprintReadOnly, Category = "Lobby")
	bool bIsReady = false;
	
	UPROPERTY(Replicated, BlueprintReadOnly, Category = "Lobby")
	int32 ReplicatedCountdownTime = -1;
	
	UFUNCTION()
	void OnRep_IsReady();
	
	void BroadcastLobbyInfo();
};
