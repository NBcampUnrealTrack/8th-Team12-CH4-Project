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
			ASGLobbyGameState* SG_PlayerState = PC->GetPlayerState<ASGLobbyGameState>();
			if (SG_PlayerState && SG_PlayerState->bIsReady)
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


void ASGLobbyGameMode::PostLogin(APlayerController* NewPlayerController)
{
	Super::PostLogin(NewPlayerController);
	
	int32 CurrentPlayers = GetNumPlayers();
	
	UE_LOG(LogTemp, Warning, TEXT("Player Joined! Current Players : %d / %d"), CurrentPlayers, TargetPlayerCount);
	NotifyAllPlayers(FString::Printf(TEXT("A player has joined. (%d/%d)"), CurrentPlayers, TargetPlayerCount));
	
	
	// 목표 인원이 채워졌고, 아직 카운트다운이 시작되지 않았다면 카운트다운 시작
	if (CurrentPlayers >= TargetPlayerCount && !bIsCountdownActive)
	{
		StartCountdown();
	}
	
}

void ASGLobbyGameMode::Logout(AController* ExitingController)
{
	// [수정 포인트] Super::Logout을 먼저 호출하여 부모 클래스가 플레이어를 리스트에서 완전히 지우도록 합니다.
	Super::Logout(ExitingController);

	// 나간 플레이어가 완전히 제외된 후의 정확한 남은 인원수를 가져옵니다.
	int32 RemainingPlayers = GetNumPlayers();

	UE_LOG(LogTemp, Warning, TEXT("Player Left! Remaining Players: %d"), RemainingPlayers);
	NotifyAllPlayers(FString::Printf(TEXT("A player has left. (%d/%d)"), RemainingPlayers, TargetPlayerCount));

	// 인원이 목표치보다 부족해졌는데 카운트다운이 진행 중이었다면 취소합니다.
	if (RemainingPlayers < TargetPlayerCount && bIsCountdownActive)
	{
		CancelCountdown();
	}
}

void ASGLobbyGameMode::OnPlayerReadyChanged()
{
	CheckReadyState();
}

void ASGLobbyGameMode::RequestChangeTeam(AController* PlayerController, ESGPlayerTeam NewTeam)
{
	if (!PlayerController) return;

	int32 RedCount = 0;
	int32 BlueCount = 0;
	int32 WaitingCount = 0;

	// GameMode는 GameState를 통해 현재 접속 중인 전체 PlayerState 명단에 바로 접근할 수 있습니다! (매우 가볍고 빠름)
	if (GameState)
	{
		for (APlayerState* PS : GameState->PlayerArray)
		{
			if (ASGMainPlayerState* SGMainPS = Cast<ASGMainPlayerState>(PS))
			{
				switch (SGMainPS->CurrentTeam)
				{
				case ESGPlayerTeam::RedTeam:     RedCount++; break;
				case ESGPlayerTeam::BlueTeam:    BlueCount++; break;
				case ESGPlayerTeam::Neutrality:  WaitingCount++; break;
				}
			}
		}
	}

	// 인원 제한 조건 체크
	bool bCanJoin = false;
	switch (NewTeam)
	{
	case ESGPlayerTeam::RedTeam:     if (RedCount < 3) bCanJoin = true; break;
	case ESGPlayerTeam::BlueTeam:    if (BlueCount < 3) bCanJoin = true; break;
	case ESGPlayerTeam::Neutrality:  if (WaitingCount < 6) bCanJoin = true; break;
	}

	// False 자리없음
	if (bCanJoin)
	{
		// Player State 맞는지 
		if (ASGLobbyPlayerState* TargetPS = PlayerController->GetPlayerState<ASGLobbyPlayerState>())
		{
			TargetPS->SetTeamInternal(NewTeam);
		}
	}
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
		// 시간이 다 되면 타이머를 끄고 인게임 레벨로 이동 처리
		GetWorld()->GetTimerManager().ClearTimer(CountdownTimerHandle);
		TransitionToGameLevel();
	}
}

void ASGLobbyGameMode::TransitionToGameLevel()
{
	NotifyAllPlayers(TEXT("Launching match"));

	// 서버 트래블을 호출하여 접속 중인 모든 클라이언트를 다음 레벨로 강제 이동시킵니다.
	UE_LOG(LogTemp, Warning,TEXT("Level Path: %s"), *GameplayLevelPath);
	GetWorld()->ServerTravel(GameplayLevelPath);
	UE_LOG(LogTemp, Warning, TEXT("Change Server"));
	
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
