// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"

#include "SoccerGame/Public/PlayerState/SGMainPlayerState.h"
#include "GameFramework/OnlineReplStructs.h" // FUniqueNetIdRepl 사용을 위해 필요
#include "Subsystems/GameInstanceSubsystem.h"
#include "SGPlayerGameInstanceSubsystem.generated.h"

/**
 * 
 */
UCLASS()
class SOCCERGAME_API USGPlayerGameInstanceSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()
	
public:
	// 특정 플레이어의 데이터를 Subsystem에 백업 (서버에서 호출)
	void SavePlayerData(const FUniqueNetIdRepl& InNetId, const FPlayerBackupData& InData);

	// 새 레벨에서 특정 플레이어의 데이터를 찾아 복원 (서버에서 호출)
	bool LoadPlayerData(const FUniqueNetIdRepl& InNetId, FPlayerBackupData& OutData);

	// 게임 오버나 세션 종료 시 백업 맵을 완전히 비우는 함수
	void ClearAllBackupData();

private:
	// Vlaue 수정된 백업 데이터 구조체
	TMap<FUniqueNetIdRepl, FPlayerBackupData> ServerBackupDataMap;
};
	
