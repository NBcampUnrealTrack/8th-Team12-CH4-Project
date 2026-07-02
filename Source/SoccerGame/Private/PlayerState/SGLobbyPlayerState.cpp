// Fill out your copyright notice in the Description page of Project Settings.


#include "PlayerState/SGLobbyPlayerState.h"
#include "GameFramework/PlayerState.h"
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
	DOREPLIFETIME(ASGLobbyPlayerState, CurrentTeam);
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
        
		// 서버 컴퍼넌트 자체에서도 값이 바뀐 것을 알리기 위해 
		// 필요하다면 GameMode에게 "이 유저 레디 상태 바뀜!" 하고 신호를 보낼 수 있습니다.
	}
}
void ASGLobbyPlayerState::OnRep_IsReady()
{
	// [클라이언트 실행] 나영 님(UI 담당)이 여기를 연동하셔야 합니다.
	// 예: 이 플레이어의 로비 슬롯 UI를 찾아 'Ready' 불빛 텍스트를 켜거나 끔
	UE_LOG(LogTemp, Log, TEXT("%s 플레이어의 레디 상태 변경: %s"), *GetPlayerName(), bIsReady ? TEXT("준비완료") : TEXT("대기중"));
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
			// 각 데이터 저장
			MainPlayerState->CurrentTeam = this->CurrentTeam;
			MainPlayerState->PlayerScore = this->LobbyScore;
			MainPlayerState->CustomPlayerName = this->CustomPlayerName;
            
			UE_LOG(LogTemp, Log, TEXT("SGLobbyPlayerState: CopyProperties 성공! 데이터가 직접 이송되었습니다. [이름: %s]"), *CustomPlayerName);
		}
	}
}



void ASGLobbyPlayerState::SetTeamInternal(const ESGPlayerTeam& SellectTeam)
{
	// 오직 서버 권한이 있을 때만 값이 변경되도록 안전장치 마련
	if (!HasAuthority()) return;

	// 실제 데이터 변경 (이 값이 바뀌면 Replicated 설정에 의해 클라이언트들에게 자동 동기화됨)
	CurrentTeam = SellectTeam;
    
	UE_LOG(LogTemp, Log, TEXT("PlayerState: %s's team is now updated to %d"), *GetPlayerName(), static_cast<int32>(CurrentTeam));
}
