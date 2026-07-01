// Fill out your copyright notice in the Description page of Project Settings.


#include "PlayerState/SGLobbyPlayerState.h"
#include "SoccerGame/Public/Instance/SGPlayerGameInstanceSubsystem.h"
#include "Net/UnrealNetwork.h" // DOREPLIFETIME 사용을 위해 필수

ASGLobbyPlayerState::ASGLobbyPlayerState()
{
	// 멀티플레이어 환경에서 이 액터가 복제되도록 설정
	bReplicates = true;
	
}

void ASGLobbyPlayerState::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	
	// 이름, 팀, 스코어 변수를 네트워크 복제 대상으로 등록
	DOREPLIFETIME(ASGLobbyPlayerState, CustomPlayerName);
	DOREPLIFETIME(ASGLobbyPlayerState, LobbyTeam);
	DOREPLIFETIME(ASGLobbyPlayerState, LobbyScore);
	
}

void ASGLobbyPlayerState::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	Super::EndPlay(EndPlayReason);
}

void ASGLobbyPlayerState::OnRep_CustomPlayerName()
{
	// 이름이 동기화되었을 때 처리할 로직 (예: 로비 UI의 닉네임 텍스트 갱신 호출 등)
	UE_LOG(LogTemp, Log, TEXT("LobbyPlayerState: 이름 동기화 완료 - %s"), *CustomPlayerName);
}

void ASGLobbyPlayerState::CopyProperties(APlayerState* NewPlayerState)
{
	Super::CopyProperties(NewPlayerState);
	
	if (HasAuthority())
	{
		// 새로 스폰된 인게임용 MainPlayerState로 캐스팅 시도
		if (ASGMainPlayerState* MainPlayerState = Cast<ASGMainPlayerState>(NewPlayerState))
		{
			// 서브시스템 없이 "구 액터 ➔ 신 액터"로 메모리 직접 복사 (순서 꼬임 0%)
			MainPlayerState->CurrentTeam = this->LobbyTeam;
			MainPlayerState->PlayerScore = this->LobbyScore;
            
			// 내장된 PlayerName과 직접 만든 CustomPlayerName 둘 다 안전하게 세팅
			MainPlayerState->SetPlayerName(this->CustomPlayerName);
            
			UE_LOG(LogTemp, Log, TEXT("SGLobbyPlayerState: CopyProperties 성공! 데이터가 직접 이송되었습니다. [이름: %s]"), *CustomPlayerName);
		}
	}
}
