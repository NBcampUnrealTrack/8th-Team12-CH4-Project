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
	DOREPLIFETIME(ASGMainPlayerState, SelectedCharacterTag);
}

void ASGMainPlayerState::BeginPlay()
{
    Super::BeginPlay();
    UE_LOG(LogTemp, Log, TEXT("[PS_Debug] MainPlayerState Start -> 서버 권한 여부: %d / 현재 데이터: [이름: %s | 팀: %s]"), 
        HasAuthority(), *CustomPlayerName, *CurrentTeamTag.ToString());
}

void ASGMainPlayerState::CopyProperties(APlayerState* NewPlayerState)
{
    Super::CopyProperties(NewPlayerState);

    if (ASGMainPlayerState* NewPS = Cast<ASGMainPlayerState>(NewPlayerState))
    {
        UE_LOG(LogTemp, Warning, TEXT("[PS_Debug] CopyProperties() 시작 (원본 데이터) -> [이름: %s | 팀: %s | 스코어: %d]"),
            *CustomPlayerName, *CurrentTeamTag.ToString(), PlayerScore);

        // 데이터 이전 실행
        NewPS->CustomPlayerName = this->CustomPlayerName;
        NewPS->CurrentTeamTag = this->CurrentTeamTag;
        NewPS->PlayerScore = this->PlayerScore;
		NewPS->SelectedCharacterTag = this->SelectedCharacterTag;

        UE_LOG(LogTemp, Warning, TEXT("[PS_Debug] CopyProperties() 완료 (사본 데이터) -> [이름: %s | 팀: %s | 스코어: %d]"),
            *NewPS->CustomPlayerName, *NewPS->CurrentTeamTag.ToString(), NewPS->PlayerScore);
    }
}

void ASGMainPlayerState::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
    if (HasAuthority() && EndPlayReason == EEndPlayReason::LevelTransition)
    {
        UE_LOG(LogTemp, Warning, TEXT("[PS_Debug] EndPlay(LevelTransition) 감지! Subsystem에 데이터를 백업합니다."));

        if (UGameInstance* GI = GetGameInstance())
        {
            if (USGPlayerGameInstanceSubsystem* DataSubsystem = GI->GetSubsystem<USGPlayerGameInstanceSubsystem>())
            {
                // 데이터 저장  
                FPlayerBackupData DataToSave;
                DataToSave.PlayerName = this->CustomPlayerName;
                DataToSave.PlayerTeam = this->CurrentTeamTag;
				DataToSave.SelectedCharacterTag = this->SelectedCharacterTag;
                DataToSave.Score = this->PlayerScore;

                UE_LOG(LogTemp, Log, TEXT("[PS_Debug] 백업 데이터 내용 -> [이름: %s | 팀: %s | 스코어: %d]"), 
                    *DataToSave.PlayerName, *DataToSave.PlayerTeam.ToString(), DataToSave.Score);

                // 서브 시스템으로 데이터 토스
                DataSubsystem->SavePlayerData(GetUniqueId(), DataToSave);
            }
        }
    }

    Super::EndPlay(EndPlayReason);
}

void ASGMainPlayerState::OnRep_CustomPlayerName()
{
    UE_LOG(LogTemp, Display, TEXT("[PS_Debug] OnRep_CustomPlayerName() 수신 -> 클라이언트 동기화 이름: %s"), *CustomPlayerName);
}
