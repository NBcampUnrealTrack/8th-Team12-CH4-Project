// Fill out your copyright notice in the Description page of Project Settings.


#include "GameState/SGLobbyGameState.h"
#include "Audio/SGInGameAudioSubsystem.h"
#include "Engine/World.h"
#include "SoccerGame/Public/PlayerController/SGLobbyPlayerController.h"
#include "SoccerGame/Public/PlayerState/SGLobbyPlayerState.h"
#include "SoccerGame/Public/UI/SGLobbyWidget.h"
#include "Net/UnrealNetwork.h"

ASGLobbyGameState::ASGLobbyGameState()
{
	// 이 액터가 네트워크를 통해 복제되도록 활성화
	bReplicates = true;
}

void ASGLobbyGameState::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(ASGLobbyGameState, bIsReady);
	DOREPLIFETIME(ASGLobbyGameState, ReplicatedCountdownTime);
}

void ASGLobbyGameState::OnRep_IsReady()
{
}

void ASGLobbyGameState::OnRep_CountdownTime()
{
	// 1. 이 함수는 클라이언트들에서 호출되므로, '로컬 플레이어 컨트롤러'를 가져옵니다.
	APlayerController* LocalPC = GetWorld()->GetFirstPlayerController();
	if (LocalPC) 
	{
		if (ASGLobbyPlayerController* LobbyPC = Cast<ASGLobbyPlayerController>(LocalPC))
		{
			LobbyPC->TimeUIUpdate(ReplicatedCountdownTime);
		}
	}
	if (USGInGameAudioSubsystem* AudioSubsystem = GetWorld()->GetSubsystem<USGInGameAudioSubsystem>())
	{
		switch (ReplicatedCountdownTime)
		{
		case 3:
			AudioSubsystem->HandleAudioEvent(ESGInGameAudioEvent::CountdownThree);
			break;
		case 2:
			AudioSubsystem->HandleAudioEvent(ESGInGameAudioEvent::CountdownTwo);
			break;
		case 1:
			AudioSubsystem->HandleAudioEvent(ESGInGameAudioEvent::CountdownFinal);
			break;
		default:
			break;
		}
	}
}

void ASGLobbyGameState::BroadcastLobbyInfo()
{
	if (!HasAuthority())
	{
		return;
	}
	// 현재 방에 있는 모든 플레이어의 최신 정보를 담을 배열 생성
	TArray<FSGPlayerLobbyInfo> NewPlayerInfos;
	for (APlayerState* BasePlayerState : PlayerArray)
	{
		ASGLobbyPlayerState* LobbyPlayerState =Cast<ASGLobbyPlayerState>(BasePlayerState);
		if (!IsValid(LobbyPlayerState))
		{
			continue;
		}

		FSGPlayerLobbyInfo PlayerInfo;

		PlayerInfo.UserName =LobbyPlayerState->GetCustomPlayerName().IsEmpty()
				? LobbyPlayerState->GetPlayerName(): LobbyPlayerState->GetCustomPlayerName();

		PlayerInfo.bIsReady =LobbyPlayerState->IsReady();

		PlayerInfo.TeamTag =LobbyPlayerState->GetTeamTag();

		NewPlayerInfos.Add(PlayerInfo);
	}

	for (FConstPlayerControllerIterator Iterator =GetWorld()->GetPlayerControllerIterator();Iterator;++Iterator)
	{
		ASGLobbyPlayerController* LobbyPlayerController =Cast<ASGLobbyPlayerController>(Iterator->Get());

		if (!IsValid(LobbyPlayerController))
		{
			continue;
		}

		LobbyPlayerController->Client_UpdateLobbyUI(NewPlayerInfos);
	}

	/*
	// GameState가 쥐고 있는 대기방 전체 인원 명단 순회
	for (APlayerState* BasePS : PlayerArray)
	{
		ASGLobbyPlayerState* LobbyPS = Cast<ASGLobbyPlayerState>(BasePS);
		if (!IsValid(BasePS))
		{
			continue;
		}
		
		if (ASGLobbyPlayerState* LobbyPS = Cast<ASGLobbyPlayerState>(BasePS))
		{
			FSGPlayerLobbyInfo Info;
			// 커스텀 이름이 비어있으면 기본 엔진 이름을, 있으면 커스텀 이름을 세팅
			Info.UserName = LobbyPS->CustomPlayerName.IsEmpty()? 
							LobbyPS->GetPlayerName():LobbyPS->CustomPlayerName;
			Info.bIsReady = LobbyPS->IsReady();
			Info.TeamTag = LobbyPS->GetTeamTag();

			// 종합 배열에 차곡차곡 추가
			NewPlayerInfos.Add(Info);
		}
	}

	// 코드가 실행 중인 내 컴퓨터의 로컬 
	if (APlayerController* PC = GetWorld()->GetFirstPlayerController())
	{
		if (PC && PC->IsLocalController())
		{
			if (ASGLobbyPlayerController* LobbyPC = Cast<ASGLobbyPlayerController>(PC))
			{
				// 묶인 전체 명단 배열
				LobbyPC->Client_UpdateLobbyUI(NewPlayerInfos);
			}
		}
	}
	 */
}
