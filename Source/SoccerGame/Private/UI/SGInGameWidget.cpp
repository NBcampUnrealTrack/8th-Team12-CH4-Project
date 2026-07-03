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

void USGInGameWidget::NativeTick(const FGeometry& MyGeometry, float InDeltaTime)
{
	Super::NativeTick(MyGeometry, InDeltaTime);
	
	ASGMainGameState* GS = Cast<ASGMainGameState>(UGameplayStatics::GetGameState(this));
	if (GS)
	{
		// 타이머 갱신
		if (Text_Timer)
		{
			int32 TotalSeconds = FMath::FloorToInt(GS->CurrentGameTime);
		
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
	
	
}

