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
    // 최초 진입 시 GameState 설정 세팅 및 대기 상태 태그 적용
    if (ASGMainGameState* SG_GameState = GetGameState<ASGMainGameState>())
    {
       SG_GameState->CurrentGameTime = TotalMatchTime;
       SG_GameState->RedTeamScore = 0;
       SG_GameState->BlueTeamScore = 0;
    }
    
    StartLoading();
}

void ASGMainGameMode::StartLoading()
{
   UE_LOG(LogTemp, Log, TEXT("[GameMode] === 로딩 및 데이터 검증 시퀀스 시작 (5초) ==="));
   CurrentLoadingTime = 0;

   // 접속한 모든 플레이어의 입력을 막고 마우스 커서 숨기기
   /*
   for (FConstPlayerControllerIterator It = GetWorld()->GetPlayerControllerIterator(); It; ++It)
   {
      if (ASGMainPlayerController* PC = Cast<ASGMainPlayerController>(It->Get()))
      {
         // 키보드/마우스 이동 입력을 무시하도록 설정
         //PC->SetInputMode(FInputModeGameOnly());
         //PC->bShowMouseCursor = false;
         //PC->SetIgnoreMoveInput(true);
         //   
         //PC->ClientMessage(TEXT("로딩 및 팀 데이터 복원 중입니다... 잠시만 기다려주세요."));
      }
   }
   */

   GetWorldTimerManager().SetTimer(
       LoadingCheckTimerHandle,
       this,
       &ASGMainGameMode::UpdateLoadingProgress,
       1.0f,
       true
   );
}

void ASGMainGameMode::UpdateLoadingProgress()
{
   CurrentLoadingTime++;
   float RemainingTime = 5 - CurrentLoadingTime;
    
   UE_LOG(LogTemp, Warning, TEXT("[GameMode] 로딩 진행 중... (%.0f초 경과 / %.0f초 남음)"), CurrentLoadingTime, RemainingTime);
   int32 UnloadedPlayers = 0;

   // 5초 동안 매초 마다 플레이어들의 로딩(데이터 복원) 상태 검션
   if (GameState)
   {
      for (APlayerState* BasePS : GameState->PlayerArray)
      {
         if (ASGMainPlayerState* MainPS = Cast<ASGMainPlayerState>(BasePS))
         {
            // 팀 태그가 아직 대기(Waiting) 상태이거나 이름이 비어있다면 로딩이 덜 된 것으로 판단
            FGameplayTag WaitingTag = FGameplayTag::RequestGameplayTag(FName("Team.Waiting"));
            if (MainPS->CustomPlayerName.IsEmpty() || MainPS->CurrentTeamTag == WaitingTag)
            {
               UnloadedPlayers++;
            }
         }
      }
   }

   if (UnloadedPlayers > 0)
   {
      UE_LOG(LogTemp, Error, TEXT("[GameMode - Loading Check] 아직 데이터 복원이 안 된 플레이어 수: %d명"), UnloadedPlayers);
   }
   else
   {
      UE_LOG(LogTemp, Log, TEXT("[GameMode - Loading Check] 모든 플레이어 데이터 복원 확인 완료!"));
   }

   // 5초가 완료되면 타이머를 끄고 게임을 시작합니다.
   if (CurrentLoadingTime >= 5)
   {
      GetWorldTimerManager().ClearTimer(LoadingCheckTimerHandle);
      StartGame();
   }
}

void ASGMainGameMode::StartGame()
{
   UE_LOG(LogTemp, Warning, TEXT("[GameMode] === 5초 대기 종료! 경기 시작! 입력을 활성화합니다. ==="));

   // 제한해두었던 플레이어들의 이동 및 시선 입력을 모두 해제
   for (FConstPlayerControllerIterator It = GetWorld()->GetPlayerControllerIterator(); It; ++It)
   {
      if (ASGMainPlayerController* PC = Cast<ASGMainPlayerController>(It->Get()))
      {
         
         //PC->SetIgnoreMoveInput(false);
         //PC->SetIgnoreLookInput(false);
         //// 호루라기 소리 사운드 재생이나 UI 알림을 넣기 좋은 타이밍입니다.
         //PC->ClientMessage(TEXT("경기 시작!! 움직일 수 있습니다!"));
      }
   }
   if (ASGMainGameState* SG_GameState = GetGameState<ASGMainGameState>())
   {
      SG_GameState->CurrentGameTime = TotalMatchTime;
      SG_GameState->RedTeamScore = 0;
      SG_GameState->BlueTeamScore = 0;
      //SG_GameState->CurrentMatchStateTag = FGameplayTag::RequestGameplayTag(FName("Match.State.WaitingToStart"));
       
      UE_LOG(LogTemp, Log, TEXT("[Debug_State] GameState 초기화 및 대기 상태 태그 설정 완료"));
   }
   //SpawnNewBall();
   GetWorldTimerManager().SetTimer(
       MatchTimerHandle,
       this,
       &ASGMainGameMode::UpdateMatchTime,
       1.0f,
       true
   );
}

void ASGMainGameMode::UpdateMatchTime()
{
    UE_LOG(LogTemp, Log, TEXT("[Debug_Call] ASGMainGameMode::UpdateMatchTime() 호출됨"));

    ASGMainGameState* SG_GameState = GetGameState<ASGMainGameState>();
    if (SG_GameState)
    {
       SG_GameState->CurrentGameTime -= 1;
       
       for (FConstPlayerControllerIterator It = GetWorld()->GetPlayerControllerIterator(); It; ++It)
       {
          if (ASGMainPlayerController* PC = Cast<ASGMainPlayerController>(It->Get()))
          {
             PC->UpdateTimerWidget(SG_GameState->CurrentGameTime);
             //SpawnNewBall();
          }
       }
       
       if (SG_GameState->CurrentGameTime <= 0.0f)
       {
          SG_GameState->CurrentGameTime = 0.0f;
          UE_LOG(LogTemp, Warning, TEXT("[Debug_State] 경기 시간 종료 조건 충족"));
          GetWorldTimerManager().ClearTimer(MatchTimerHandle);
          
          //EndMatch();
       }
    }
}

void ASGMainGameMode::OnGoalScored(FGameplayTag GoalTeamTag)
{
   if (!HasAuthority())
   {
      return;
   }
   static int32 Count = 0;
   Count++;

   UE_LOG(LogTemp, Warning,
       TEXT("OnGoalScored %d : %s"),
       Count,
       *GoalTeamTag.ToString());
   ASGMainGameState* SG_GameState = GetGameState<ASGMainGameState>();
   if (!SG_GameState)
   {
      UE_LOG(LogTemp, Warning, TEXT("[MainGame] No SGMainGameState"));
      return;
   }
   // 골 연출 동안 시간을 멈추기 위해 넣은 코드. 필요없다면 삭제하되, 유지한다면 RestartRound쪽에서 타이머 다시 재생해야함.
   //GetWorldTimerManager().ClearTimer(MatchTimerHandle);
   //GetWorldTimerManager().ClearTimer(RoundRestartTimerHandle);
	
   // bIsRedTeamGoal : Red팀의 득점일 때
   if (GoalTeamTag == FGameplayTag::RequestGameplayTag(FName("Team.Red")))
   {
      SG_GameState->BlueTeamScore++;
      //SG_GameState->OnRep_UpdateScore();
      GEngine->AddOnScreenDebugMessage(-1, 10.f, FColor::Red,
         FString::Printf(TEXT("Goal! RedTeamScore: %d"), SG_GameState->RedTeamScore));
   }
   // !bIsRedTeamGoal : Blue팀의 득점일 때
   else if (GoalTeamTag == FGameplayTag::RequestGameplayTag(FName("Team.Blue")))
   {
      SG_GameState->RedTeamScore++;
      //SG_GameState->OnRep_UpdateScore();
   }
	
   //{
   //   if (ASGMainPlayerController* PC = Cast<ASGMainPlayerController>(It->Get()))
   //   {
   //      PC->UpdateScoreWidget( SG_GameState->BlueTeamScore,SG_GameState->RedTeamScore);
   //   }
   //}
   // 공 제거
   if (IsValid(SpawnedBall))
   {
      SpawnedBall->Destroy();
      SpawnedBall = nullptr;
   }
	
   // 승리 조건 체크
   const bool bRedTeamWon = SG_GameState->RedTeamScore >= ScoreToWin;
   const bool bBlueTeamWon = SG_GameState->BlueTeamScore >= ScoreToWin;
	
   if (bRedTeamWon || bBlueTeamWon)
   {
      EndMatch();
      return;
   }
	
   // RestartRound 이전 시간 딜레이
   GetWorldTimerManager().SetTimer(
      RoundRestartTimerHandle,
      this,
      &ASGMainGameMode::RestartRound,
      GoalRestartDelay,
      false
   );
}

void ASGMainGameMode::SpawnNewBall()
{
   if (!HasAuthority())
   {
      return;
   }
	
   if (BallClass == nullptr)
   {
      return;
   }
	
   // 공이 남아 있으면 제거
   if (IsValid(SpawnedBall))
   {
      SpawnedBall->Destroy();
      SpawnedBall = nullptr;
   }
	
   FTransform SpawnTransform;
	
   // 레벨에 배치된 BallSpawn 태그 부착된 액터 찾기
   TArray<AActor*> FoundSpawnPoints;
   UGameplayStatics::GetAllActorsWithTag(GetWorld(), BallSpawnTag, FoundSpawnPoints);
	
   if (FoundSpawnPoints.Num() > 0 && IsValid(FoundSpawnPoints[0]))
   {
      SpawnTransform = FoundSpawnPoints[0]->GetActorTransform();
   }

   // 못찾았으면 임시로 원점 할당
   else
   {
      SpawnTransform = FTransform(FRotator::ZeroRotator, FVector::ZeroVector);
   }
	
   FActorSpawnParameters SpawnParams;
   SpawnParams.Owner = this;
   SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
	
   SpawnedBall = GetWorld()->SpawnActor<AActor>(
      BallClass,
      SpawnTransform,
      SpawnParams
   );
   GEngine->AddOnScreenDebugMessage(
            -1, 
            5.0f, 
            FColor::Green, 
            FString::Printf(TEXT("⚽ 새로운 공이 스폰되었습니다! 위치: %s"), *SpawnedBall->GetActorLocation().ToString()));
}

AActor* ASGMainGameMode::ChoosePlayerStart_Implementation(AController* Player)
{
    // 🌟 호출 확인용 단순 디버그 로그 한 줄
    UE_LOG(LogTemp, Log, TEXT("[Debug_Call] ASGMainGameMode::ChoosePlayerStart_Implementation() 호출됨"));

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

       AssignedInitialPlayerStarts.Add(Player, CandidateStart);
       return CandidateStart;
    }

    return Super::ChoosePlayerStart_Implementation(Player);
}
void ASGMainGameMode::HandleSeamlessTravelPlayer(AController*& Controller)
{
    UE_LOG(LogTemp, Log, TEXT("[Debug_Call] ASGMainGameMode::HandleSeamlessTravelPlayer() 호출됨"));

    Super::HandleSeamlessTravelPlayer(Controller);

    if (APlayerController* PC = Cast<APlayerController>(Controller))
    {
       // 로딩 중에는 조작을 완전히 차단
       PC->SetInputMode(FInputModeGameOnly());
       PC->bShowMouseCursor = false;
       PC->DisableInput(PC); 
    }
}
void ASGMainGameMode::EndMatch()
{
  
   
}

void ASGMainGameMode::RestartRound()
{
}
