// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerState.h"
#include "SoccerGame/Public/PlayerState/SGMainPlayerState.h" // ESGPlayerTeam 사용을 위해 포함
#include "SGLobbyPlayerState.generated.h"

/**
 * 
 */
UCLASS()
class SOCCERGAME_API ASGLobbyPlayerState : public APlayerState
{
	GENERATED_BODY()
	
public:
	ASGLobbyPlayerState();
	
	// 멀티플레이 동기화를 위한 변수 등록 함수
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

protected:
	// 레벨 이동으로 파괴 시 호출 (데이터 백업)
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

public:
	// ----------------------------------------------------
	// 로비용 데이터 변수들
	// ----------------------------------------------------

	// [추가] 리플리케이션되는 플레이어 이름 변수 (값이 변경되면 OnRep_CustomPlayerName 호출)
	UPROPERTY(ReplicatedUsing = OnRep_CustomPlayerName, BlueprintReadOnly, Category = "Lobby Settings")
	FString CustomPlayerName = TEXT("UnknownPlayer");

	UPROPERTY(Replicated, BlueprintReadOnly, Category = "Lobby Settings")
	ESGPlayerTeam LobbyTeam = ESGPlayerTeam::Neutrality;

	UPROPERTY(Replicated, BlueprintReadOnly, Category = "Lobby Settings")
	int32 LobbyScore = 0;

protected:
	// [추가] 이름이 서버로부터 동기화되었을 때 클라이언트에서 실행될 함수 (UI 갱신 등에 활용)
	UFUNCTION()
	void OnRep_CustomPlayerName();
	
	void CopyProperties(APlayerState* NewPlayerState);
	
};
