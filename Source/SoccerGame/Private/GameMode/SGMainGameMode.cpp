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

	PlayerControllerClass = ASGMainPlayerController::StaticClass();
	PlayerStateClass = ASGMainPlayerState::StaticClass();
	GameStateClass = ASGMainGameState::StaticClass();
}

void ASGMainGameMode::BeginPlay()
{
	Super::BeginPlay();
	
	// PlayerController 보다 먼저 호출됨
	
	UE_LOG(LogTemp, Warning, TEXT("In Main Server"));
	
	// 최초 진입 시 GameState 설정 세팅 및 대기 상태 태그 적용
	if (ASGMainGameState* SG_GameState = GetGameState<ASGMainGameState>())
	{
		SG_GameState->CurrentGameTime = TotalMatchTime;
		SG_GameState->RedTeamScore = 0;
		SG_GameState->BlueTeamScore = 0;
		SG_GameState->CurrentMatchStateTag = FGameplayTag::RequestGameplayTag(FName("Match.State.WaitingToStart"));
	}
	
	StartLoading();
}

void ASGMainGameMode::HandleSeamlessTravelPlayer(AController*& Controller)
{
	Super::HandleSeamlessTravelPlayer(Controller);

	if (APlayerController* PC = Cast<APlayerController>(Controller))
	{
		// 로딩 중에는 조작을 완전히 차단
		PC->SetInputMode(FInputModeGameOnly());
		PC->bShowMouseCursor = false;
		PC->DisableInput(PC); 
	}
}

void ASGMainGameMode::StartLoading()
{
	// 시작 전 초기화
	 // 여기서 스폰을 할까?
	// 여기서 스폰하는 방식으로 변경 - PlayerState 복사가 제대로 이루어지고 있지 않았음
	//BeginPlay -> StartLoading 호출 -> 0.5초마다 StartGame에서 준비 상태 check -> 준비되면 Start
	UE_LOG(LogTemp, Warning, TEXT("[MainGame] StartLoading 시작"));

	GetWorldTimerManager().SetTimer(
	   LoadingCheckTimerHandle,
	   this,
	   &ASGMainGameMode::UpdateLoadingProgress, // 내부 로직 검사용 함수 분리
	   UpdateLoadingTime, 
	   true
	);
}

void ASGMainGameMode::UpdateLoadingProgress()
{
	// 1. 지정한 로딩 시간 채우기
	if (LodingTime < UpdateStartTime)
	{
		LodingTime += UpdateLoadingTime;
       
		// [나영님 파트 참고] 여기에 클라이언트 레벨 UI 게이지를 올려주는 등의 
		// UI Update RPC 로직을 구현하시면 됩니다.
		return;
	}

	// 2. 시간이 다 찼다면 플레이어들의 PlayerState 복사가 완료되었는지 검증
	for (FConstPlayerControllerIterator It = GetWorld()->GetPlayerControllerIterator(); It; ++It)
	{
		APlayerController* PC = It->Get();
		if (!PC || !PC->GetPlayerState<ASGMainPlayerState>())
		{
			UE_LOG(LogTemp, Warning, TEXT("[MainGame] 로딩 시간은 끝났으나 PlayerState 복사 대기 중..."));
			return;
		}
	}

	// 3. 조건 완전 충족 시 로딩 타이머를 안전하게 끄고 게임 시작!
	LodingTime = 0.0f;
	GetWorldTimerManager().ClearTimer(LoadingCheckTimerHandle);
	StartGame();
}


void ASGMainGameMode::StartGame()
{
	// 실제 게임 시작 을 담당하는 부분
	
	UE_LOG(LogTemp, Warning, TEXT("[MainGame] All player states ready. StartGame"));
	// 공 Spawn
	SpawnNewBall();
	
	for (FConstPlayerControllerIterator It = GetWorld()->GetPlayerControllerIterator(); It; ++It)
	{
		APlayerController* PC = It->Get();
		if (!PC || !PC->GetPlayerState<ASGMainPlayerState>())
		{
			UE_LOG(LogTemp, Warning, TEXT("[MainGame] Waiting for SGMainPlayerState..."));
			return;
		}
	}
	
	// 조작 잠금 해제
	for (FConstPlayerControllerIterator It = GetWorld()->GetPlayerControllerIterator(); It; ++It)
	{
		if (APlayerController* PC = It->Get())
		{
			PC->EnableInput(PC);
		}
	}

	
	// State 데이터 초기화
	if (ASGMainGameState* SG_GameState = GetGameState<ASGMainGameState>())
	{
		SG_GameState->CurrentGameTime = TotalMatchTime;
		SG_GameState->RedTeamScore = 0;
		SG_GameState->BlueTeamScore = 0;
		SG_GameState->CurrentMatchStateTag = FGameplayTag::RequestGameplayTag(FName("Match.State.WaitingToStart"));
	}
	//BeginPlay의 매치 타이머 부분을 StartGame으로 옮김
	GetWorldTimerManager().SetTimer(
		MatchTimerHandle,
		this,
		&ASGMainGameMode::UpdateMatchTime,
		UpdateLoadingTime,
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
		SG_GameState->CurrentGameTime -= 1;
		
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
