// Fill out your copyright notice in the Description page of Project Settings.


#include "GameMode/SGLobbyGameMode.h"
#include "SoccerGame/Public/PlayerController/SGLobbyPlayerController.h"
#include "SoccerGame/Public/GameState/SGLobbyGameState.h"
#include "SoccerGame/Public/PlayerState/SGLobbyPlayerState.h"
#include "GameFramework/PlayerController.h"
#include "GameFramework/GameStateBase.h"
#include "Kismet/GameplayStatics.h"

ASGLobbyGameMode::ASGLobbyGameMode()
{
	bUseSeamlessTravel = true;
	// 수정한 멀티플레이어 대응 전용 클래스들을 기본 클래스로 바인딩
	PlayerControllerClass = ASGLobbyPlayerController::StaticClass();
	PlayerStateClass = ASGLobbyPlayerState::StaticClass();
	GameStateClass = ASGLobbyGameState::StaticClass();
}

void ASGLobbyGameMode::BeginPlay()
{
	Super::BeginPlay();
	CurrentCountdownTime = CountdownDuration;
}

void ASGLobbyGameMode::CheckReadyState()
{
	int32 TotalPlayers = 0;
	int32 ReadyPlayersCount = 0;

	// 1. 월드에 있는 모든 PlayerState를 순회하며 레디 상태를 전수조사합니다.
	for (FConstPlayerControllerIterator It = GetWorld()->GetPlayerControllerIterator(); It; ++It)
	{
		APlayerController* PC = It->Get();
		if (PC)
		{
			TotalPlayers++;
			// [버그 수정] 기존에 PlayerState 자리에 GameState를 넣고 캐스팅하려던 치명적인 오류를 수정했습니다.
			ASGLobbyPlayerState* SG_PlayerState = PC->GetPlayerState<ASGLobbyPlayerState>();
			if (SG_PlayerState && SG_PlayerState->IsReady())
			{
				ReadyPlayersCount++;
			}
		}
	}

	UE_LOG(LogTemp, Log, TEXT("로비 현황 파악 - 총 인원: %d/%d , 준비 완료: %d"), TotalPlayers, TargetPlayerCount, ReadyPlayersCount);

	// 2. [Ready 확인 / 취소 분기 규칙 판단]
	// 최소 인원(6명)이 충족되었고, 그 인원이 '전원' 레디 상태인지 확인합니다.
	if (TotalPlayers >= TargetPlayerCount && ReadyPlayersCount == TotalPlayers)
	{
		// 모든 조건 충족 (Ready 확인 로직 완료 -> 시작 카운트다운 가동)
		if (!GetWorldTimerManager().IsTimerActive(CountdownTimerHandle))
		{
			UE_LOG(LogTemp, Log, TEXT("모든 플레이어가 준비되었습니다! 5초 후 경기를 시작합니다."));
			GetWorldTimerManager().SetTimer(CountdownTimerHandle, this, &ASGLobbyGameMode::TransitionToGameLevel, 5.0f, false);
		}
	}
	else
	{
		// 한 명이라도 레디를 취소했거나(bIsReady = false), 인원이 부족한 경우
		// (Ready 취소 로직 자동 처리)
		if (GetWorldTimerManager().IsTimerActive(CountdownTimerHandle))
		{
			GetWorldTimerManager().ClearTimer(CountdownTimerHandle);
			UE_LOG(LogTemp, Warning, TEXT("준비되지 않은 플레이어가 있어 게임 시작 카운트다운이 취소되었습니다."));
		}
	}
}

int32 ASGLobbyGameMode::GetTeamCount(const FGameplayTag& TeamTag) const
{
	int32 Count = 0;
    
	// 기존에 인게임용 레벨용 State인 ASGMainPlayerState로 잘못 캐스팅하던 부분을 로비용 ASGLobbyPlayerState로 정상 매칭했습니다.
	if (GameState)
	{
		for (APlayerState* PS : GameState->PlayerArray)
		{
			if (ASGLobbyPlayerState* LobbyPS = Cast<ASGLobbyPlayerState>(PS))
			{
				if (LobbyPS->GetTeamTag() == TeamTag)
				{
					Count++;
				}
			}
		}
	}
	return Count;
}


void ASGLobbyGameMode::PostLogin(APlayerController* NewPlayerController)
{
	Super::PostLogin(NewPlayerController);
	int32 CurrentPlayers = GetNumPlayers();
    
	// 접속했는지만 확인
	UE_LOG(LogTemp, Warning, TEXT("Player Joined! Current Players : %d " ), CurrentPlayers);
}
	

void ASGLobbyGameMode::Logout(AController* ExitingController)
{
	Super::Logout(ExitingController);
	
	int32 RemainingPlayers = GetNumPlayers();

	UE_LOG(LogTemp, Warning, TEXT("Player Left! Remaining Players: %d"), RemainingPlayers);
	NotifyAllPlayers(FString::Printf(TEXT("A player has left. (%d/%d)"), RemainingPlayers, TargetPlayerCount));

	// 확인해볼 의향이 있음.
	// 인원이 목표치보다 부족해졌는데 카운트다운이 진행 중이었다면 취소합니다.
	if (RemainingPlayers < TargetPlayerCount && bIsCountdownActive)
	{
		CancelCountdown();
	}

	// 퇴장 완료 후 전체 UI 갱신 유도
	if (ASGLobbyGameState* GS = GetGameState<ASGLobbyGameState>())
	{
		GS->BroadcastLobbyInfo();
	}
}
void ASGLobbyGameMode::OnPlayerReadyChanged()
{
	CheckReadyState();
}

void ASGLobbyGameMode::ProcessChangeTeamRequest(APlayerController* TargetPC, const FGameplayTag& RequestedTeamTag)
{
	// 타겟이 아니면 return
	if (!TargetPC) return;

	//PlayerState도 아니면 return 
	ASGLobbyPlayerState* TargetPS = TargetPC->GetPlayerState<ASGLobbyPlayerState>();
	if (!TargetPS) return;

	// 이미 요청한 팀에 소속되어 있다면 무시
	if (TargetPS->GetTeamTag() == RequestedTeamTag) return;

	// 최대 허용 정원 체크 (FGameplayTag 조건 분기 구조로 전환)
	FGameplayTag WaitingTag = FGameplayTag::RequestGameplayTag(FName("Team.Waiting"));
	int32 MaxCapacity = (RequestedTeamTag == WaitingTag) ? 6 : 3;

	if (GetTeamCount(RequestedTeamTag) >= MaxCapacity)
	{
		UE_LOG(LogTemp, Warning, TEXT("서버 차단: 해당 팀(%s)은 가득 차 자리가 없습니다."), *RequestedTeamTag.ToString());
		return;
	}

	TargetPS->SetTeamInternal(RequestedTeamTag);
	TargetPS->SetReadyState(false);
}

void ASGLobbyGameMode::StartCountdown()
{
	bIsCountdownActive = true;
	CurrentCountdownTime = CountdownDuration;

	NotifyAllPlayers(FString::Printf(TEXT("Lobby is full! Game starts in %d seconds..."), CurrentCountdownTime));

	// 1초마다 TickCountdown 함수를 실행시키는 타이머 설정
	GetWorld()->GetTimerManager().SetTimer(
		CountdownTimerHandle,
		this,
		&ASGLobbyGameMode::TickCountdown,
		1.0f,
		true
	);
}

void ASGLobbyGameMode::CancelCountdown()
{
	bIsCountdownActive = false;

	// 카운트다운 타이머 초기화(해제)
	GetWorld()->GetTimerManager().ClearTimer(CountdownTimerHandle);
	NotifyAllPlayers(TEXT("Player left. Waiting for players to refill..."));
}

void ASGLobbyGameMode::TickCountdown()
{
	CurrentCountdownTime--;
	UE_LOG(LogTemp, Warning, TEXT("Current Counttime %d"), CurrentCountdownTime);

	if (CurrentCountdownTime > 0)
	{
		NotifyAllPlayers(FString::Printf(TEXT("Game starts in %d..."), CurrentCountdownTime));
	}
	else
	{
		GetWorld()->GetTimerManager().ClearTimer(CountdownTimerHandle);
	}
}

void ASGLobbyGameMode::TransitionToGameLevel()
{
	NotifyAllPlayers(TEXT("Launching match"));

	UE_LOG(LogTemp, Warning,TEXT("Level Path: %s"), *GameplayLevelPath);
	GetWorld()->ServerTravel(GameplayLevelPath);
	
}

void ASGLobbyGameMode::NotifyAllPlayers(const FString& Message)
{
	// 월드 내의 모든 플레이어 컨트롤러를 순회하며 메시지를 전달합니다.
	for (FConstPlayerControllerIterator It = GetWorld()->GetPlayerControllerIterator(); It; ++It)
	{
		APlayerController* PC = It->Get();
		if (IsValid(PC))
		{
			// 클라이언트 화면 좌측 하단에 메시지를 띄우는 함수 (디버깅/테스트용으로 유용)
			PC->ClientMessage(Message);
		}
	}
}
