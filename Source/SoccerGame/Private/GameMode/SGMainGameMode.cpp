// Fill out your copyright notice in the Description page of Project Settings.


#include "GameMode/SGMainGameMode.h"
#include "SoccerGame/Public/GameState/SGMainGameState.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/PlayerStart.h"
#include "Level/SGPlayerStart.h"
#include "SoccerGame/Public/PlayerState/SGMainPlayerState.h"
#include "GameFramework/PlayerController.h"
#include "PlayerController/SGMainPlayerController.h"

ASGMainGameMode::ASGMainGameMode()
{
	// GameMode는 오직 서버에만 존재하므로 복제(Replicate)할 필요가 없습니다.
	bReplicates = false;

	// Main Game Mode 용 PlayerController, PlayerState, GameState
	PlayerControllerClass = ASGMainPlayerController::StaticClass();
	PlayerStateClass = ASGMainPlayerState::StaticClass();
	GameStateClass = ASGMainGameState::StaticClass();

	UE_LOG(LogTemp, Warning, TEXT("In Main Server"));
	
}

void ASGMainGameMode::BeginPlay()
{
	Super::BeginPlay();
	
	// PlayerController 보다 먼저 호출됨
	
	UE_LOG(LogTemp, Warning, TEXT("In Main Server"));
	
	// 2. 초기 맵 세팅 및 공 스폰 부분 StartGame 부분으로 옮김


	// 3. GameState(전광판) 초기값 세팅 및 시작 명령 -> 시작 명령 부분은 실제 게임 시작하는 부분으로 옮김.
	ASGMainGameState* SG_GameState = GetGameState<ASGMainGameState>();
	if (SG_GameState)
	{
		SG_GameState->CurrentGameTime = TotalMatchTime;
		SG_GameState->RedTeamScore = 0;
		SG_GameState->BlueTeamScore = 0;
		// 직접 들고 있지 않고, GameState의 상태 변수를 변경하여 클라이언트에 전파!
		SG_GameState->CurrentMatchState = ESGMatchState::InProgress;
	}

	//  1초 간격 인게임 타이머 가동 -> 이 부분은 실제 StartGame 부분으로 이동
	StartLoading();
}

void ASGMainGameMode::HandleSeamlessTravelPlayer(AController*& C)
{
	UE_LOG(LogTemp, Warning, TEXT("[MainGame] HandleSeamlessTravelPlayer Before: %s"), *GetNameSafe(C));
	Super::HandleSeamlessTravelPlayer(C);
	UE_LOG(LogTemp, Warning, TEXT("[MainGame] HandleSeamlessTravelPlayer After: %s"), *GetNameSafe(C));

	if (APlayerController* PC = Cast<APlayerController>(C))
	{
		PC->SetInputMode(FInputModeGameOnly());
		PC->bShowMouseCursor = false;
	}
}

void ASGMainGameMode::StartLoading()
{
	// 시작 전 초기화
	 // 여기서 스폰을 할까?
	// 여기서 스폰하는 방식으로 변경 - PlayerState 복사가 제대로 이루어지고 있지 않았음
	//BeginPlay -> StartLoading 호출 -> 0.5초마다 StartGame에서 준비 상태 check -> 준비되면 Start
	UE_LOG(LogTemp, Warning, TEXT("[MainGame] StartLoading"));

	GetWorldTimerManager().SetTimer(
		LoadingCheckTimerHandle,
		this,
		&ASGMainGameMode::StartGame,
		0.5f,
		true
	);
}

void ASGMainGameMode::StartGame()
{
	// 실제 게임 시작 을 담당하는 부분
	// UpdateMatchTime 
	for (FConstPlayerControllerIterator It = GetWorld()->GetPlayerControllerIterator(); It; ++It)
	{
		APlayerController* PC = It->Get();
		if (!PC || !PC->GetPlayerState<ASGMainPlayerState>())
		{
			UE_LOG(LogTemp, Warning, TEXT("[MainGame] Waiting for SGMainPlayerState..."));
			return;
		}
	}

	GetWorldTimerManager().ClearTimer(LoadingCheckTimerHandle);

	UE_LOG(LogTemp, Warning, TEXT("[MainGame] All player states ready. StartGame"));

	SpawnNewBall();

	if (ASGMainGameState* SG_GameState = GetGameState<ASGMainGameState>())
	{
		SG_GameState->CurrentGameTime = TotalMatchTime;
		SG_GameState->RedTeamScore = 0;
		SG_GameState->BlueTeamScore = 0;
		SG_GameState->CurrentMatchState = ESGMatchState::InProgress;
	}
	//BeginPlay의 매치 타이머 부분을 StartGame으로 옮김
	GetWorldTimerManager().SetTimer(
		MatchTimerHandle,
		this,
		&ASGMainGameMode::UpdateMatchTime,
		1.0f,
		true
	);
		

}


void ASGMainGameMode::OnGoalScored(bool bIsRedTeamGoal)
{

}

void ASGMainGameMode::EndMatch()
{
}

void ASGMainGameMode::RestartRound()
{
}

void ASGMainGameMode::SpawnNewBall()
{
}

void ASGMainGameMode::UpdateMatchTime()
{
	ASGMainGameState* SG_GameState = GetGameState<ASGMainGameState>();
	if (SG_GameState)
	{
		SG_GameState->CurrentGameTime -= 1.0f;
		
		if (SG_GameState->CurrentGameTime <= 0.0f)
		{
			SG_GameState->CurrentGameTime = 0.0f;
			GetWorldTimerManager().ClearTimer(MatchTimerHandle);
			
			EndMatch();
		}
	}
}

AActor* ASGMainGameMode::ChoosePlayerStart_Implementation(AController* Player)
{
	if (!Player)
	{
		return Super::ChoosePlayerStart_Implementation(Player);
	}

	if (ASGPlayerStart** AssignedStart = AssignedInitialPlayerStarts.Find(Player))
	{
		if (IsValid(*AssignedStart))
		{
			return *AssignedStart;
		}
	}

	ASGMainPlayerState* MainPS = Player->GetPlayerState<ASGMainPlayerState>();
	if (!MainPS)
	{
		UE_LOG(LogTemp, Warning, TEXT("[MainSpawn] %s has no SGMainPlayerState. Using default PlayerStart."), *GetNameSafe(Player));
		return Super::ChoosePlayerStart_Implementation(Player);
	}

	const FGameplayTag RedTeamTag = FGameplayTag::RequestGameplayTag(FName("Team.Red"));
	const FGameplayTag BlueTeamTag = FGameplayTag::RequestGameplayTag(FName("Team.Blue"));

	ETeamId DesiredTeam = ETeamId::None;
	if (MainPS->CurrentTeamTag == RedTeamTag)
	{
		DesiredTeam = ETeamId::Red;
	}
	else if (MainPS->CurrentTeamTag == BlueTeamTag)
	{
		DesiredTeam = ETeamId::Blue;
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("[MainSpawn] %s has unsupported team %s. Using default PlayerStart."),
			*GetNameSafe(Player),
			*MainPS->CurrentTeamTag.ToString());
		return Super::ChoosePlayerStart_Implementation(Player);
	}

	TArray<AActor*> FoundPlayerStarts;
	UGameplayStatics::GetAllActorsOfClass(GetWorld(), ASGPlayerStart::StaticClass(), FoundPlayerStarts);
	TArray<ASGPlayerStart*> TeamPlayerStarts;

	for (AActor* FoundActor : FoundPlayerStarts)
	{
		ASGPlayerStart* SGPlayerStart = Cast<ASGPlayerStart>(FoundActor);
		if (!SGPlayerStart || !SGPlayerStart->bUseForInitialSpawn || SGPlayerStart->TeamId != DesiredTeam)
		{
			continue;
		}

		TeamPlayerStarts.Add(SGPlayerStart);
	}

	TeamPlayerStarts.Sort([](const ASGPlayerStart& A, const ASGPlayerStart& B)
	{
		return A.SpawnIndex < B.SpawnIndex;
	});

	for (ASGPlayerStart* CandidateStart : TeamPlayerStarts)
	{
		if (AssignedInitialPlayerStarts.FindKey(CandidateStart))
		{
			continue;
		}

		UE_LOG(LogTemp, Warning,
			TEXT("[MainSpawn] Player=%s / Team=%s / Start=%s / SpawnIndex=%d"),
			*GetNameSafe(Player),
			*MainPS->CurrentTeamTag.ToString(),
			*CandidateStart->GetName(),
			CandidateStart->SpawnIndex);

		AssignedInitialPlayerStarts.Add(Player, CandidateStart);
		return CandidateStart;
	}

	UE_LOG(LogTemp, Error, TEXT("[MainSpawn] No available PlayerStart for %s / Team=%s. Using default PlayerStart."),
		*GetNameSafe(Player),
		*MainPS->CurrentTeamTag.ToString());

	return Super::ChoosePlayerStart_Implementation(Player);
}
