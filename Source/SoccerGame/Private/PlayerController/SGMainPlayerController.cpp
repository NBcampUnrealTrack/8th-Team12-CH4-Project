// Fill out your copyright notice in the Description page of Project Settings.


#include "PlayerController/SGMainPlayerController.h"
#include "GameState/SGMainGameState.h"
#include "Blueprint/UserWidget.h"
#include "UI/SGInGameWidget.h"
#include "SoccerGame/Public/Instance/SGPlayerGameInstanceSubsystem.h"
#include "SoccerGame/Public/PlayerState/SGMainPlayerState.h" // PlayerState 검증을 위해 추가



void ASGMainPlayerController::BeginPlay()
{
    Super::BeginPlay();
    if (IsLocalController())
    {
        if (IsValid(UIMainGameWidgetClass) == true)
        {
            UIMainGameWidgetInstance = CreateWidget<USGInGameWidget>(this, UIMainGameWidgetClass); 
            if (IsValid(UIMainGameWidgetInstance) == true)
            {
                UIMainGameWidgetInstance->AddToViewport();
			
                FInputModeUIOnly Mode;
                Mode.SetWidgetToFocus(UIMainGameWidgetInstance->GetCachedWidget());
                SetInputMode(Mode);
			
                bShowMouseCursor = true;
            }
        }
    }
    UE_LOG(LogTemp, Warning, TEXT("[PC_Debug] BeginPlay() 호출됨 -> 로컬 소유 여부: %d / 서버 권한 여부: %d"), IsLocalController(), HasAuthority());
    if (HasAuthority())
    {
        GetWorldTimerManager().SetTimer(LoadDataTimerHandle, this, &ASGMainPlayerController::LoadPlayerData, 0.2f, true);
    }
    // 초기 상태 설정을 위한 호출 (로딩 중이라면 마우스를 뺏기지 않음)
    ApplyGameInputMode();
}

void ASGMainPlayerController::OnPossess(APawn* InPawn)
{
    Super::OnPossess(InPawn);
    
    UE_LOG(LogTemp, Warning, TEXT("[PC_Debug] OnPossess() 호출됨 -> 소유한 캐릭터(Pawn): %s"), *GetNameSafe(InPawn));

    // 데이터가 서버측에 안전하게 안착했는지 중간 검션
    if (ASGMainPlayerState* PS = GetPlayerState<ASGMainPlayerState>())
    {
        UE_LOG(LogTemp, Log, TEXT("[PC_Debug] OnPossess 시점 PlayerState 데이터 -> [이름: %s | 팀: %s | 스코어: %d]"),
            *PS->CustomPlayerName, *PS->CurrentTeamTag.ToString(), PS->PlayerScore);
    }
    else
    {
        UE_LOG(LogTemp, Error, TEXT("[PC_Debug] OnPossess 시점에 아직 PlayerState가 유효하지 않습니다."));
    }
    
    ApplyGameInputMode();
}
void ASGMainPlayerController::AcknowledgePossession(APawn* P)
{
    Super::AcknowledgePossession(P);

    UE_LOG(LogTemp, Warning,
       TEXT("[MainPC] AcknowledgePossession: %s / Pawn=%s / IsLocal=%d"),
       *GetNameSafe(this),
       *GetNameSafe(P),
       IsLocalController());

    ApplyGameInputMode();
}
void ASGMainPlayerController::LoadPlayerData()
{
    ASGMainPlayerState* MainPS = GetPlayerState<ASGMainPlayerState>();
    if (MainPS == nullptr)
    {
        return;
    }

    // 이미 복원이 완료된 상태(Team.Waiting이 아니고 이름이 채워짐)라면 중복 처리 방지를 위해 true 반환
    const FGameplayTag WaitingTag = FGameplayTag::RequestGameplayTag(FName("Team.Waiting"));
    if (!MainPS->CustomPlayerName.IsEmpty() && MainPS->CurrentTeamTag != WaitingTag)
    {
        return; 
    }

    // 2. 고유 ID 유효성 확인
    FUniqueNetIdRepl UniqueId = MainPS->GetUniqueId();
    if (!UniqueId.IsValid()) return ;

    // 3. GameInstanceSubsystem 접근
    UGameInstance* GI = GetGameInstance();
    if (!GI) return ;

    USGPlayerGameInstanceSubsystem* DataSubsystem = GI->GetSubsystem<USGPlayerGameInstanceSubsystem>();
    if (!DataSubsystem) return ;

    FPlayerBackupData LoadedData;
    
    // 4. 데이터를 서브시스템에서 로드 시도
    if (DataSubsystem->LoadPlayerData(UniqueId, LoadedData))
    {
        MainPS->CustomPlayerName = LoadedData.PlayerName;
        MainPS->CurrentTeamTag = LoadedData.PlayerTeam;
        MainPS->PlayerScore = LoadedData.Score;

        MainPS->ForceNetUpdate();
        GetWorldTimerManager().ClearTimer(LoadDataTimerHandle); 
    }
}

void ASGMainPlayerController::UpdateScoreWidget(int32 BlueTeam,int32 RedTeam)
{
    FString NetRole = HasAuthority() ? TEXT("SERVER") : TEXT("CLIENT");

    UE_LOG(LogTemp, Warning,
        TEXT("[%s] %s | Local=%d | Pawn=%s"),
        *NetRole,
        *FString(__FUNCTION__),
        IsLocalController(),
        *GetNameSafe(GetPawn()));
    
    if (!IsLocalController()) return;

    if (UIMainGameWidgetInstance)
    {
        UIMainGameWidgetInstance->UpdateScores(BlueTeam, RedTeam);
    }
   
}


void ASGMainPlayerController::UpdateTimerWidget_Implementation(int32 NewTime)
{
    // 로컬 클라이언트(실제 모니터가 있는 화면)에서만 실행되도록 안전장치
    if (!IsLocalController()) return;

    if (UIMainGameWidgetInstance)
    {
        UE_LOG(LogTemp, Warning, TEXT("UpdateTimerWidget : %d"), NewTime);
        UIMainGameWidgetInstance->UpdateTimerUI(NewTime);
    }
    else
    {
        UE_LOG(LogTemp, Error, TEXT("[UI Error] UIMainGameWidgetInstance가 아직 생성되지 않았습니다!"));
    }
}

void ASGMainPlayerController::ApplyGameInputMode()
{
    UE_LOG(LogTemp, Warning,
        TEXT("[MainPC] Input mode check: %s / IsLocal=%d / HasAuthority=%d / Pawn=%s"),
        *GetNameSafe(this),
        IsLocalController(),
        HasAuthority(),
        *GetNameSafe(GetPawn()));

    if (!IsLocalController())
    {
        return;
    }
    //// GameState에서 현재 매치가 시작되었는지(로딩이 끝났는지) 상태 체크
    //ASGMainGameState* GS = GetWorld()->GetGameState<ASGMainGameState>();
    //if (GS)
    //{
    //	// 예시: 아직 로딩 단계(MatchState가 Loading 같은 것)라면 
    //	// 게임 모드로 강제 전환하지 않고, UI를 조작할 수 있는 UI 전용 모드를 유지합니다.
    //	// if (GS->CurrentMatchStateTag == FGameplayTag::RequestGameplayTag(TEXT("MatchState.Loading"))) return;
    //}

    // 로딩이 끝나고 진짜 인게임 플레이를 시작할 때만 아래 코드가 수행되도록 제어해야 합니다.
    SetInputMode(FInputModeGameOnly());
    bShowMouseCursor = false;

    UE_LOG(LogTemp, Warning, TEXT("[MainPC] Local game input mode applied: %s"), *GetNameSafe(this));
}
