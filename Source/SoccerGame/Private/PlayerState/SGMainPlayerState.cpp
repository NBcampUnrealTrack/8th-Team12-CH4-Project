// Fill out your copyright notice in the Description page of Project Settings.


#include "PlayerState/SGMainPlayerState.h"
#include "SoccerGame/Public/Instance/SGPlayerGameInstanceSubsystem.h"// 우리가 만든 서브시스템 헤더
#include "Net/UnrealNetwork.h"     // DOREPLIFETIME 매크로 사용을 위해 필요

ASGMainPlayerState::ASGMainPlayerState()
{
    bReplicates = true;
}

// 개별 변수들을 네트워크 동기화 목록에 등록
void ASGMainPlayerState::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);

    // 변수별로 변경 사항이 생길 때만 패킷을 보냅니다 (최적화)
    DOREPLIFETIME(ASGMainPlayerState, CurrentTeam);
    DOREPLIFETIME(ASGMainPlayerState, PlayerScore);
}

void ASGMainPlayerState::BeginPlay()
{
    Super::BeginPlay();
	UE_LOG(LogTemp, Log, TEXT("MainPlayerState Start"));

    UE_LOG(LogTemp, Log, TEXT("MainPlayerState:kakakakaka Data restored for  (Team: %d, Score: %d)") 
    , static_cast<int32>(CurrentTeam), PlayerScore);
}

void ASGMainPlayerState::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
    // 심리스 트래블 등으로 레벨이 전환될 때만 서버가 데이터를 구조체로 포장하여 백업합니다.
    if (HasAuthority() && EndPlayReason == EEndPlayReason::LevelTransition)
    {
        if (UGameInstance* GI = GetGameInstance())
        {
            if (USGPlayerGameInstanceSubsystem* DataSubsystem = GI->GetSubsystem<USGPlayerGameInstanceSubsystem>())
            {
                // 보낼 때는 개별 변수들을 구조체(가방) 하나로 이쁘게 포장합니다.
                FPlayerBackupData DataToSave;
                DataToSave.PlayerName = GetPlayerName();
                DataToSave.PlayerTeam = this->CurrentTeam;
                DataToSave.Socre = this->PlayerScore;

                // 서브시스템에 포장된 가방 전달
                DataSubsystem->SavePlayerData(GetUniqueId(), DataToSave);
            }
        }
    }

    Super::EndPlay(EndPlayReason);
}