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

    //동기화 등록
    DOREPLIFETIME(ASGMainPlayerState, CustomPlayerName);
    DOREPLIFETIME(ASGMainPlayerState, CurrentTeamTag);
    DOREPLIFETIME(ASGMainPlayerState, PlayerScore);
}

void ASGMainPlayerState::BeginPlay()
{
    Super::BeginPlay();
	UE_LOG(LogTemp, Log, TEXT("MainPlayerState Start"));

    
}

void ASGMainPlayerState::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
    if (HasAuthority() && EndPlayReason == EEndPlayReason::LevelTransition)
    {
        if (UGameInstance* GI = GetGameInstance())
        {
            if (USGPlayerGameInstanceSubsystem* DataSubsystem = GI->GetSubsystem<USGPlayerGameInstanceSubsystem>())
            {
                // 데이터 저장  
                FPlayerBackupData DataToSave;
                DataToSave.PlayerName = this->CustomPlayerName;
                DataToSave.PlayerTeam = this->CurrentTeamTag;
                DataToSave.Score = this->PlayerScore;

                // 서브 시스템으로 데이터 토스
                DataSubsystem->SavePlayerData(GetUniqueId(), DataToSave);
            }
        }
    }

    Super::EndPlay(EndPlayReason);
}

void ASGMainPlayerState::OnRep_CustomPlayerName()
{
   
}
