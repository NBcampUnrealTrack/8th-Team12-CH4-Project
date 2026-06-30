// Fill out your copyright notice in the Description page of Project Settings.


#include "GameMode/SGLobbyGameMode.h"
#include "SoccerGame/Public/PlayerController/SGLobbyPlayerController.h"
#include "SoccerGame/Public/GameState/SGLobbyGameState.h"
#include "SoccerGame/Public/PlayerState/SGPlayerState.h"
#include "GameFramework/PlayerController.h"
#include "GameFramework/GameStateBase.h"
#include "Kismet/GameplayStatics.h"

ASGLobbyGameMode::ASGLobbyGameMode()
{
	bUseSeamlessTravel = true;
	// 수정한 멀티플레이어 대응 전용 클래스들을 기본 클래스로 바인딩
	PlayerControllerClass = ASGLobbyPlayerController::StaticClass();
	PlayerStateClass = ASGPlayerState::StaticClass();
}
void ASGLobbyGameMode::BeginPlay()
{
	Super::BeginPlay();
	CurrentCountdownTime = CountdownDuration;
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
	NotifyAllPlayers(TEXT("Launching match..."));

	// 서버 트래블을 호출하여 접속 중인 모든 클라이언트를 다음 레벨로 강제 이동시킵니다.
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
