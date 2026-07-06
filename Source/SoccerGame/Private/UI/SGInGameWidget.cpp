// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/SGInGameWidget.h"
#include "UI/SGScoreBoardWidget.h"
#include "Components/TextBlock.h"
#include "GameState/SGMainGameState.h"
#include "Kismet/GameplayStatics.h"

void USGInGameWidget::NativeConstruct()
{
	Super::NativeConstruct();
	
}

void USGInGameWidget::UpdateTimerUI(int32 NewTime)
{
	if (!IsValid(WBP_Timer)) return;

	// 초 단위 시간을 MM:SS 형식으로 변환
	int32 Minutes = NewTime / 60;
	int32 Seconds = NewTime % 60;

	// FString::Printf를 사용하여 두 자리 숫자로 패딩 ("%02d")
	FString TimeString = FString::Printf(TEXT("%02d:%02d"), Minutes, Seconds);

	WBP_Timer->SetText(FText::FromString(TimeString));
}

void USGInGameWidget::UpdateScores(int32 RedScore, int32 BlueScore)
{
	// 점수 변경이 발생했을 때만 업데이트 (불필요한 호출 방지)
	if (LastRedTeamScore != RedScore || LastBlueTeamScore != BlueScore)
	{
		LastRedTeamScore = RedScore;
		LastBlueTeamScore = BlueScore;

		if (IsValid(WBP_ScoreBoardWidget))
		{
			WBP_ScoreBoardWidget->UpdateScores(RedScore, BlueScore);
		}
	}
}

void USGInGameWidget::NativeTick(const FGeometry& MyGeometry, float InDeltaTime)
{
	Super::NativeTick(MyGeometry, InDeltaTime);
	/*
	ASGMainGameState* GS = Cast<ASGMainGameState>(UGameplayStatics::GetGameState(this));
	if (GS)
	{
		// 타이머 갱신
		if (Text_Timer)
		{
			int32 TotalSeconds = GS->CurrentGameTime;
		
			int32 Minutes = TotalSeconds / 60;
			int32 Seconds = TotalSeconds % 60;
		
			FString TimeString = FString::Printf(TEXT("%d:%02d"), Minutes, Seconds);
			Text_Timer->SetText(FText::FromString(TimeString));	
		}
		
		if (WBP_ScoreBoard && (LastBlueTeamScore != GS->BlueTeamScore || LastRedTeamScore != GS->RedTeamScore))
		{
			LastBlueTeamScore = GS->BlueTeamScore;
			LastRedTeamScore = GS->RedTeamScore;
			
			WBP_ScoreBoard->UpdateScores(LastBlueTeamScore, LastRedTeamScore);
		}
	}        
	 */
}

