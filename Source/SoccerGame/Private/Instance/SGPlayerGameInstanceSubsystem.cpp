// Fill out your copyright notice in the Description page of Project Settings.


#include "Instance/SGPlayerGameInstanceSubsystem.h"

void USGPlayerGameInstanceSubsystem::SavePlayerData(const FUniqueNetIdRepl& InNetId, const FPlayerBackupData& InData)
{
	if (InNetId.IsValid())
	{
		ServerBackupDataMap.Emplace(InNetId, InData);
	}
}

bool USGPlayerGameInstanceSubsystem::LoadPlayerData(const FUniqueNetIdRepl& InNetId, FPlayerBackupData& OutData)
{
	if (InNetId.IsValid())
	{
		// Map에서 해당 플레이어의 NetId 검색
		if (ServerBackupDataMap.Contains(InNetId))
		{
			// 참조 변수를 통해 구조체 데이터를 통째로 넘겨줌
			OutData = ServerBackupDataMap[InNetId];

			// 불러온 데이터는 메모리 관리 및 중복 복원 방지를 위해 제거
			ServerBackupDataMap.Remove(InNetId);
        
			UE_LOG(LogTemp, Log, TEXT("Subsystem: Player Data Loaded. [Name: %s]"), *OutData.PlayerName);
			return true;
		}
	}

	UE_LOG(LogTemp, Warning, TEXT("Subsystem: Failed to find backup data for Player."));
	return false;
}

void USGPlayerGameInstanceSubsystem::ClearAllBackupData()
{
	ServerBackupDataMap.Empty();
	UE_LOG(LogTemp, Log, TEXT("Subsystem: Backup Data Map Cleared."));
}
