// Fill out your copyright notice in the Description page of Project Settings.


#include "Instance/SGPlayerGameInstanceSubsystem.h"

void USGPlayerGameInstanceSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);
	// 엔진 시작 시 이 로그가 찍혀야 서브시스템이 정상 등록된 것입니다.
	UE_LOG(LogTemp, Warning, TEXT("[SGSubsystem] 플레이어 데이터 백업 서브시스템이 성공적으로 초기화되었습니다."));
}

void USGPlayerGameInstanceSubsystem::Deinitialize()
{
	// 메모리 누수 방지를 위해 게임 종료 시 맵을 비워줍니다.
	ClearAllBackupData();
	Super::Deinitialize();
}

void USGPlayerGameInstanceSubsystem::SavePlayerData(const FUniqueNetIdRepl& InNetId, const FPlayerBackupData& InData)
{
	if (InNetId.IsValid())
	{
		ServerBackupDataMap.Add(InNetId, InData);
	}
}

bool USGPlayerGameInstanceSubsystem::LoadPlayerData(const FUniqueNetIdRepl& InNetId, FPlayerBackupData& OutData)
{
	if (InNetId.IsValid())
	{
		// 언리얼에서 가장 권장하는 'Find' 함수를 통해 단 한 번만 조회하고 포인터로 안전하게 접근합니다.
		if (FPlayerBackupData* FoundData = ServerBackupDataMap.Find(InNetId))
		{
			// 참조 변수를 통해 구조체 데이터를 통째로 넘겨줌
			OutData = *FoundData;

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
