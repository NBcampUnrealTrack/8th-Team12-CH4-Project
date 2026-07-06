// Fill out your copyright notice in the Description page of Project Settings.


#include "PlayerController/SGMainPlayerController.h"
#include "GameState/SGMainGameState.h"
#include "Blueprint/UserWidget.h"
#include "UI/SGInGameWidget.h"

void ASGMainPlayerController::BeginPlay()
{
	Super::BeginPlay();
	
	// 🌟 [추가] 오직 이 컴퓨터의 주인인 '로컬 플레이어'일 때만 UI를 생성하도록 제한합니다!
	if (!IsLocalController())
	{
		return; 
	}
	// 초기 UI 생성 
	if (IsValid(UIMainGameWidgetClass))
	{
		// 실제 프로젝트 위젯 타입(USGInGameWidget)으로 안전하게 생성 및 저장
		UIMainGameWidgetInstance = CreateWidget<USGInGameWidget>(this, UIMainGameWidgetClass); 
		if (IsValid(UIMainGameWidgetInstance))
		{
			UIMainGameWidgetInstance->AddToViewport();
            
			// 처음에는 UI 조작이 가능하도록 마우스 커서 활성화 및 포커스 설정
			FInputModeUIOnly Mode;
			Mode.SetWidgetToFocus(UIMainGameWidgetInstance->GetCachedWidget());
			SetInputMode(Mode);
			bShowMouseCursor = true;
            
			// UI가 늦게 켜졌을 수도 있으므로,현재 GameState에 이미 저장된 시간이 있다면 즉시 반영
			if (ASGMainGameState* GS = GetWorld()->GetGameState<ASGMainGameState>())
			{
				// UI 업데이트 
				//UIMainGameWidgetInstance->UpdateTimerUI(GS->CurrentGameTime);
			}
		}
	}

	// 초기 상태 설정을 위한 호출 (로딩 중이라면 마우스를 뺏기지 않음)
	ApplyGameInputMode();
}

void ASGMainPlayerController::OnPossess(APawn* InPawn)
{
	Super::OnPossess(InPawn);
	
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
