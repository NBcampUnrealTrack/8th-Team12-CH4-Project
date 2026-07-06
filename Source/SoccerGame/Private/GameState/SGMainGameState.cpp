// Fill out your copyright notice in the Description page of Project Settings.


#include "GameState/SGMainGameState.h"
#include "Net/UnrealNetwork.h"
#include "Kismet/GameplayStatics.h"
#include "PlayerController/SGMainPlayerController.h"
#include "UI/SGInGameWidget.h"

ASGMainGameState::ASGMainGameState()
{
}

void ASGMainGameState::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	
		
	DOREPLIFETIME(ASGMainGameState, CurrentMatchStateTag);
	DOREPLIFETIME(ASGMainGameState, RedTeamScore);
	DOREPLIFETIME(ASGMainGameState, BlueTeamScore);
	DOREPLIFETIME(ASGMainGameState, CurrentGameTime);
}

void ASGMainGameState::OnRep_MatchState()
{
}

void ASGMainGameState::OnRep_UpdateScore()
{
	// 레드팀이든 블루팀이든 점수가 복제되어 내려오면 이 함수가 실행됩니다.
	APlayerController* PC = UGameplayStatics::GetPlayerController(this, 0);
	if (!PC) return;

	ASGMainPlayerController* SG_PC = Cast<ASGMainPlayerController>(PC);
	if (IsValid(SG_PC))
	{
		//USGInGameWidget* InGameWidget = SG_PC->GetInGameWidget();
		//if (IsValid(InGameWidget))
		//{
		//	// 최신 레드팀 점수와 블루팀 점수를 UI에 통째로 갱신합니다.
		//	InGameWidget->UpdateScores(RedTeamScore, BlueTeamScore);
		//}
	}
}
void ASGMainGameState::OnRep_UpdateTime()
{
	// 1. 이 함수는 서버에서 CurrentGameTime이 복제되어 내려올 때 '클라이언트'에서 실행됩니다.
	// 현재 이 컴퓨터(로컬 컴퓨터)를 쓰고 있는 플레이어의 컨트롤러를 안전하게 가져옵니다.
	APlayerController* PC = UGameplayStatics::GetPlayerController(this, 0);
	if (!PC) return;

	// 2. 프로젝트에서 정의한 축구 게임 전용 플레이어 컨트롤러로 캐스팅합니다.
	ASGMainPlayerController* SG_PC = Cast<ASGMainPlayerController>(PC);
	if (!SG_PC) return;
	
	// 3. 컨트롤러 내부 혹은 HUD에 나영님이 생성해서 들고 계실 메인 인게임 위젯을 참조합니다.
	// (※ SGMainPlayerController 내부에 GetInGameWidget() 같은 Getter 함수가 있다고 가정합니다.)
	//USGInGameWidget* InGameWidget = SG_PC->GetInGameWidget();
	//if (InGameWidget)
	//{
	//	// 4. 인게임 위젯에 작성해 둔 타이머 갱신 함수를 호출하여 화면을 업데이트합니다.
	//	// 정수형(int32) 시간 데이터를 그대로 넘겨줍니다.
	//	InGameWidget->UpdateTimerUI(CurrentGameTime);
	//}
}
