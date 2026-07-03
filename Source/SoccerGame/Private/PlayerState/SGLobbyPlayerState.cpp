// Fill out your copyright notice in the Description page of Project Settings.


#include "PlayerState/SGLobbyPlayerState.h"
#include "GameFramework/PlayerState.h"
#include "SoccerGame/Public/PlayerState/SGMainPlayerState.h"
#include "SoccerGame/Public/PlayerController/SGLobbyPlayerController.h"
#include "SoccerGame/Public/GameState/SGLobbyGameState.h"
#include "SoccerGame/UI/SGLobbyWidget.h"
#include "Net/UnrealNetwork.h" 

ASGLobbyPlayerState::ASGLobbyPlayerState()
{
	// 멀티플레이어 환경에서 이 액터가 복제되도록 설정
	bReplicates = true;
	bIsReady = false;
}

void ASGLobbyPlayerState::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	// 이름, 팀 태그, 스코어, 레디 상태 변수를 네트워크 복제 대상으로 등록
	DOREPLIFETIME(ASGLobbyPlayerState, CustomPlayerName);
	DOREPLIFETIME(ASGLobbyPlayerState, CurrentTeamTag);
	DOREPLIFETIME(ASGLobbyPlayerState, LobbyScore);
	DOREPLIFETIME(ASGLobbyPlayerState, bIsReady);
}

void ASGLobbyPlayerState::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	Super::EndPlay(EndPlayReason);
}

void ASGLobbyPlayerState::SetReadyState(bool bNewReadyState)
{
	if (HasAuthority())
	{
		bIsReady = bNewReadyState;
	}
}

void ASGLobbyPlayerState::SetTeamInternal(const FGameplayTag& SelectTeamTag)
{
	if (HasAuthority())
	{
		CurrentTeamTag = SelectTeamTag;
	}
}
void ASGLobbyPlayerState::OnRep_IsReady()
{
	UE_LOG(LogTemp, Log, TEXT("%s 플레이어의 레디 상태 변경 완료!"), *GetPlayerName());

	// 공용 허브인 GameState를 가져와서 전체 명단 갱신을 요청합니다.
	if (ASGLobbyGameState* GS = GetWorld()->GetGameState<ASGLobbyGameState>())
	{
		GS->BroadcastLobbyInfo();
	}
}

void ASGLobbyPlayerState::OnRep_ChangeTeam()
{
	UE_LOG(LogTemp, Log, TEXT("%s 플레이어의 팀 태그 변경 완료: %s"), *GetPlayerName(), *CurrentTeamTag.ToString());

	// 팀이 바뀌었을 때도 마찬가지로 전체 명단 갱신을 요청합니다.
	if (ASGLobbyGameState* GS = GetWorld()->GetGameState<ASGLobbyGameState>())
	{
		GS->BroadcastLobbyInfo();
	}
}

void ASGLobbyPlayerState::OnRep_CustomPlayerName()
{
	UE_LOG(LogTemp, Log, TEXT("LobbyPlayerState: 이름 동기화 완료 - %s"), *CustomPlayerName);

	if (ASGLobbyGameState* GS = GetWorld()->GetGameState<ASGLobbyGameState>())
	{
		//GS->BroadcastLobbyInfo();
	}
}

void ASGLobbyPlayerState::CopyProperties(APlayerState* NewPlayerState)
{
	Super::CopyProperties(NewPlayerState);
	
	if (HasAuthority())
	{
		// 새로 스폰된 인게임용 MainPlayerState로 캐스팅 시도
		if (ASGMainPlayerState* MainPlayerState = Cast<ASGMainPlayerState>(NewPlayerState))
		{
			// 각 데이터 저장
			MainPlayerState->CurrentTeamTag = this->CurrentTeamTag;
			MainPlayerState->PlayerScore = this->LobbyScore;
			MainPlayerState->CustomPlayerName = this->CustomPlayerName;
            
			UE_LOG(LogTemp, Log, TEXT("SGLobbyPlayerState: CopyProperties 성공 [이름: %s]"), *CustomPlayerName);
		}
	}
}
