// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "SoccerGame/Public/PlayerState/SGLobbyPlayerState.h"
#include "GameplayTagContainer.h" 
#include "SGLobbyGameMode.generated.h"

/**
 * 
 */
UCLASS()
class SOCCERGAME_API ASGLobbyGameMode : public AGameModeBase
{
	GENERATED_BODY()
public:
	ASGLobbyGameMode();
	
	// 플레이어가 서버에 완전히 접속했을 때 호출
	virtual void PostLogin(APlayerController* NewPlayerController) override;
	// 플레이어가 접속을 종료하거나 나갔을 때 호출
	virtual void Logout(AController* ExitingController) override;
	
	// PlayerController가 서버 RPC를 통해 호출할 레디 상태 업데이트 함수
	void OnPlayerReadyChanged();
	
	void ProcessChangeTeamRequest(APlayerController* TargetPC, const FGameplayTag& RequestedTeamTag);
	
protected:
	virtual void BeginPlay() override;
	
	// 모든 유저의 레디 상태를 확인하고, 조건 만족 시 매치를 시작하는 함수
	void CheckReadyState();
private:
	int32 GetTeamCount(const FGameplayTag& TeamTag) const;
	
	// 카운트다운을 시작하는 함수
	void StartCountdown();

	// 카운트다운을 취소하는 함수 (인원 미달 시)
	void CancelCountdown();

	// 타이머에 의해 1초마다 반복 호출되며 카운트다운을 처리하는 함수
	void TickCountdown();

	// 지정된 실제 게임 레벨로 모든 플레이어를 데리고 이동 (ServerTravel)
	void TransitionToGameLevel();

	// 모든 접속자 화면에 알림 메시지를 띄우는 편의용 함수
	void NotifyAllPlayers(const FString& Message);
	
	void UpdateGameStateCountdown(int32 NewTime);
	
private:
    // 게임 시작에 필요한 목표(최대) 인원수 (예: 2명)
    UPROPERTY(EditDefaultsOnly, Category = "Lobby Settings")
    int32 TargetPlayerCount = 2;
	
    // 인원이 가득 찼을 때 대기할 시간 (초)
    UPROPERTY(EditDefaultsOnly, Category = "Lobby Settings")
    int32 CountdownDuration = 4;
	
    // 현재 남은 카운트다운 시간
    int32 CurrentCountdownTime = 0;
	
    // 카운트다운 처리를 위한 타이머 핸들
    FTimerHandle CountdownTimerHandle;
	
    // 이동할 실제 게임 플레이 맵의 경로
    UPROPERTY(EditDefaultsOnly, Category = "Lobby Settings")
    FString GameplayLevelPath = TEXT("/Game/SoccerGame/Maps/PlayBase");
	
    // 카운트다운이 현재 진행 중인지 여부
    bool bIsCountdownActive = false;
};
