// Fill out your copyright notice in the Description page of Project Settings.


#include "PlayerState/SGLobbyPlayerState.h"
#include "GameFramework/PlayerState.h"
#include "SoccerGame/Public/PlayerState/SGMainPlayerState.h"
#include "SoccerGame/Public/PlayerController/SGLobbyPlayerController.h"
#include "SoccerGame/Public/GameState/SGLobbyGameState.h"
#include "SoccerGame/Public/UI/SGLobbyWidget.h"
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
	DOREPLIFETIME_CONDITION(ASGLobbyPlayerState, SelectedCharacterTag, COND_OwnerOnly);
}

void ASGLobbyPlayerState::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	Super::EndPlay(EndPlayReason);
}

void ASGLobbyPlayerState::SetCustomPlayerName(const FString& NewPlayerName)
{
	
	if (!HasAuthority())
	{
		UE_LOG(LogTemp,Warning,
			TEXT("[LobbyPS] 클라이언트에서 이름 변경을 시도했기 때문에 거부되었습니다."));

		return;
	}

	//TrimStartAndEnd 앞뒤 공백 지우는 함수 
	const FString TrimmedPlayerName =NewPlayerName.TrimStartAndEnd();

	if (CustomPlayerName == TrimmedPlayerName)
	{
		return;
	}

	// 커스텀 이름 변경
	CustomPlayerName = TrimmedPlayerName;

	// APlayerState가 기본적으로 사용하는 PlayerName도
	// 동일한 값으로 변경합니다.
	SetPlayerName(TrimmedPlayerName);
	OnRep_CustomPlayerName();
	// 가능한 한 빠르게 클라이언트에 복제합니다.
	ForceNetUpdate();
}
void ASGLobbyPlayerState::SetReadyState(bool bNewReadyState)
{
	if (!HasAuthority())
	{
		return ;
	}
	if (bIsReady == bNewReadyState)
	{
		return;
	}
	bIsReady = bNewReadyState;
	OnRep_IsReady();
	ForceNetUpdate();
}

void ASGLobbyPlayerState::SetTeamInternal(const FGameplayTag& SelectTeamTag)
{
	if (HasAuthority())
	{
		CurrentTeamTag = SelectTeamTag;
		OnRep_ChangeTeam();
	}
}

void ASGLobbyPlayerState::SetSelectedCharacterInternal(const FGameplayTag& NewCharacterTag)
{
	if (!HasAuthority() || SelectedCharacterTag == NewCharacterTag)
	{
		return;
	}

	SelectedCharacterTag = NewCharacterTag;
	ForceNetUpdate();
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

	if (!HasAuthority())
	{
		return;
	}
	
	// 팀이 바뀌었을 때도 마찬가지로 전체 명단 갱신을 요청합니다.
	if (ASGLobbyGameState* GS = GetWorld()->GetGameState<ASGLobbyGameState>())
	{
		GS->BroadcastLobbyInfo();
	}
}

void ASGLobbyPlayerState::OnRep_CustomPlayerName()
{
	UE_LOG(LogTemp, Log, TEXT("LobbyPlayerState: 이름 동기화 완료 - %s"), *CustomPlayerName);
	if (!HasAuthority())
	{
		return;
	}
	ASGLobbyGameState* LobbyGameState =GetWorld()? 
	GetWorld()->GetGameState<ASGLobbyGameState>(): nullptr;
	if (IsValid(LobbyGameState))
	{
		LobbyGameState->BroadcastLobbyInfo();
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
			MainPlayerState->SelectedCharacterTag = this->SelectedCharacterTag;
            
			UE_LOG(LogTemp, Log, TEXT("SGLobbyPlayerState: CopyProperties 성공 [이름: %s]"), *CustomPlayerName);
		}
	}
}
