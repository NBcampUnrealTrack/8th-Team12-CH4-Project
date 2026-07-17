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

UClass* ASGMainGameMode::GetDefaultPawnClassForController_Implementation(AController* InController)
{
   if (InController)
   {
      if (const ASGMainPlayerState* MainPS = InController->GetPlayerState<ASGMainPlayerState>())
      {
         if (const TSubclassOf<ACharacter>* CharacterClass = CharacterCatalog.Find(MainPS->SelectedCharacterTag))
         {
            if (*CharacterClass)
            {
               return CharacterClass->Get();
            }
         }
      }
   }

   return Super::GetDefaultPawnClassForController_Implementation(InController);
}

void ASGMainGameMode::ReturnToMainMenu()
{
   if (!HasAuthority())
   {
      return;
   }

   UWorld* World = GetWorld();
   if (!IsValid(World))
   {
      return;
   }
   if (MainMenuLevelPath.IsEmpty())
   {
      UE_LOG(LogTemp,Error,TEXT("[GameMode] MainMenuLevelPath가 비어 있습니다."));
      return;
   }


   bIsReturningToMainMenu = true;


   UE_LOG(LogTemp,Warning,TEXT("[GameMode] 모든 플레이어를 메인 메뉴로 이동합니다: %s"),*MainMenuLevelPath);

   World->ServerTravel(MainMenuLevelPath);
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
   CurrentLoadingTime = 0;
   
   SetAllPlayersGameInputEnabled(false);


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
   float RemainingTime = LoadingDuration - CurrentLoadingTime;
    
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
            if (!MainPS)
            {
               ++UnloadedPlayers;
               continue;
            }

            if (MainPS->CustomPlayerName.IsEmpty() ||
                MainPS->CurrentTeamTag == WaitingTag)
            {
               ++UnloadedPlayers;
            }
         }
      }
   }

  
   // 3초가 완료되면 타이머를 끄고 게임을 시작합니다.
   if (CurrentLoadingTime >= LoadingDuration)
   {
      GetWorldTimerManager().ClearTimer(LoadingCheckTimerHandle);
      StartGame();
   }
}

void ASGMainGameMode::StartGame()
{
   UE_LOG(LogTemp, Warning, TEXT("[GameMode] === 5초 대기 종료! 경기 시작! 입력을 활성화합니다. ==="));

   SetAllPlayersGameInputEnabled(true);

   if (ASGMainGameState* SG_GameState = GetGameState<ASGMainGameState>())
   {
      SG_GameState->CurrentGameTime = TotalMatchTime;
      SG_GameState->RedTeamScore = 0;
      SG_GameState->BlueTeamScore = 0;
      UpdateAllPlayerScoreWidget();
      //SG_GameState->CurrentMatchStateTag = FGameplayTag::RequestGameplayTag(FName("Match.State.WaitingToStart"));
       
      UE_LOG(LogTemp, Log, TEXT("[Debug_State] GameState 초기화 및 대기 상태 태그 설정 완료"));
   }
   SpawnNewBall();
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

    ASGMainGameState* SG_GameState = GetGameState<ASGMainGameState>();
    if (SG_GameState)
    {
       SG_GameState->CurrentGameTime -= 1;
       
       for (FConstPlayerControllerIterator It = GetWorld()->GetPlayerControllerIterator(); It; ++It)
       {
          if (ASGMainPlayerController* PC = Cast<ASGMainPlayerController>(It->Get()))
          {
             PC->UpdateTimerWidget(SG_GameState->CurrentGameTime);
          }
       }
       
       if (SG_GameState->CurrentGameTime <= 0.0f)
       {
          SG_GameState->CurrentGameTime = 0.0f;
          GetWorldTimerManager().ClearTimer(MatchTimerHandle);
          EndMatch();
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

   UE_LOG(LogTemp, Warning,TEXT("OnGoalScored %d : %s"),Count,*GoalTeamTag.ToString());
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
   }
   else if (GoalTeamTag == FGameplayTag::RequestGameplayTag(FName("Team.Blue")))
   {
      SG_GameState->RedTeamScore++;
   }
   
   UpdateAllPlayerScoreWidget();
	
   if (IsValid(SpawnedBall))
   {
      SpawnedBall->Destroy();
      SpawnedBall = nullptr;
   }
   SpawnNewBall();
   //Player리스폰 추가 
   // 승리 조건 체크
   
   if (!EndScoreMatch())
   {
      return ;
   }
   EndMatch();
   
   /*
   // RestartRound 이전 시간 딜레이
   GetWorldTimerManager().SetTimer(
      RoundRestartTimerHandle,
      this,
      &ASGMainGameMode::RestartRound,
      GoalRestartDelay,
      false
   );
    */
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
            FString::Printf(TEXT(" 새로운 공이 스폰되었습니다! 위치: %s"), *SpawnedBall->GetActorLocation().ToString()));
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
   if (!HasAuthority())
   {
      return;
   }

   WinTeamCheck();
   GetWorldTimerManager().ClearTimer(MatchTimerHandle);
   GetWorldTimerManager().ClearTimer(LoadingCheckTimerHandle);
   GetWorldTimerManager().ClearTimer(RoundRestartTimerHandle);
   
   // 모든 플레이어 조작 차단
   SetAllPlayersGameInputEnabled(false);
   // 마우스 입력 true

   // SpawnBall  제거 
   if (IsValid(SpawnedBall))
   {
      SpawnedBall->Destroy();
      SpawnedBall = nullptr;
   }
   
   // 결과 UI 출력
   
   for (FConstPlayerControllerIterator It =GetWorld()->GetPlayerControllerIterator();It;++It)
   {
      ASGMainPlayerController* MainPC =Cast<ASGMainPlayerController>(It->Get());

      if (!IsValid(MainPC))
      {
         continue;
      }

      MainPC->bShowMouseCursor = true;
      MainPC->Client_ShowResultUI();
   }
}

bool ASGMainGameMode::EndScoreMatch()
{
   ASGMainGameState* SG_GameState = GetGameState<ASGMainGameState>();
   if (SG_GameState == nullptr)
   {
      return false;
   }
   // 스코어 체크해서 확인
   
   if (SG_GameState->BlueTeamScore >= ScoreToWin || SG_GameState->RedTeamScore >= ScoreToWin )
      return true;
   return false;
}
void ASGMainGameMode::WinTeamCheck()
{
   ASGMainGameState* SG_GameState = GetGameState<ASGMainGameState>();
   if (SG_GameState == nullptr)
   {
      return ;
   }
   if (SG_GameState->BlueTeamScore >= ScoreToWin || SG_GameState->BlueTeamScore > SG_GameState->RedTeamScore )
   {
      WinTeamTag = FGameplayTag::RequestGameplayTag(FName("Match.Result.BlueTeamWin"));
   }
   else if (SG_GameState->RedTeamScore >= ScoreToWin || SG_GameState->RedTeamScore > SG_GameState->BlueTeamScore )
   {
      WinTeamTag = FGameplayTag::RequestGameplayTag(FName("Match.Result.RedTeamWin"));
   }
   else
   {
      WinTeamTag = FGameplayTag::RequestGameplayTag(FName("Match.Result.Draw"));
   }
}
void ASGMainGameMode::RestartRound()
{
}

void ASGMainGameMode::RespawnAllPlayers()
{
   if (!HasAuthority())
   {
      return;
   }

   UWorld* World = GetWorld();
   if (!IsValid(World))
   {
      return;
   }

   for (FConstPlayerControllerIterator It =World->GetPlayerControllerIterator();It;++It)
   {
      AController* Controller = It->Get();

      if (!IsValid(Controller))
      {
         continue;
      }

      APawn* Pawn = Controller->GetPawn();

      if (!IsValid(Pawn))
      {
         continue;
      }

      ASGPlayerStart** FoundStart =AssignedInitialPlayerStarts.Find(Controller);

      if (!FoundStart || !IsValid(*FoundStart))
      {
         UE_LOG(
             LogTemp,
             Warning,
             TEXT("[ResetPosition] 배정된 PlayerStart 없음: %s"),
             *GetNameSafe(Controller)
         );

         continue;
      }

      ASGPlayerStart* PlayerStart = *FoundStart;

      Pawn->SetActorLocationAndRotation(PlayerStart->GetActorLocation(),PlayerStart->GetActorRotation(),
          false,nullptr,ETeleportType::TeleportPhysics);
   }
}


void ASGMainGameMode::SetAllPlayersGameInputEnabled(bool bEnableInput)
{
   UWorld* World = GetWorld();
   if (!World)
   {
      return;
   }

   for (FConstPlayerControllerIterator It =World->GetPlayerControllerIterator();It;++It)
   {
      ASGMainPlayerController* PC =Cast<ASGMainPlayerController>(It->Get());

      if (!IsValid(PC))
      {
         continue;
      }
      PC->Client_SetGameInputEnabled(bEnableInput);
   }
}

void ASGMainGameMode::UpdateAllPlayerScoreWidget()
{
   ASGMainGameState* SG_GameState = GetGameState<ASGMainGameState>();

   if (!SG_GameState)
   {
      return;
   }

   for (FConstPlayerControllerIterator It = GetWorld()->GetPlayerControllerIterator(); It; ++It)
   {
      if (ASGMainPlayerController* PC = Cast<ASGMainPlayerController>(It->Get()))
      {
         PC->UpdateScoreWidget(SG_GameState->BlueTeamScore,SG_GameState->RedTeamScore);
      }
   }
}
