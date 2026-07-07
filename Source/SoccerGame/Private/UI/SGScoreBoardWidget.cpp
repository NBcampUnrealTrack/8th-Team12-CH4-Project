// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/SGScoreBoardWidget.h"
#include "Components/TextBlock.h"


void USGScoreBoardWidget::UpdateScores(int32 BlueScore, int32 RedScore)
{	
	if (Text_BlueScore)
	{
		Text_BlueScore->SetText(FText::AsNumber(BlueScore));
		//UE_LOG(LogTemp, Warning, TEXT("Blue Text = %s"),
		//	*Text_BlueScore->GetText().ToString());
	}

	if (Text_RedScore)
	{
		Text_RedScore->SetText(FText::AsNumber(RedScore));
		//UE_LOG(LogTemp, Warning, TEXT("Red Text = %s"),
		//	*Text_RedScore->GetText().ToString());
	}
	UE_LOG(LogTemp,Log,TEXT("UpdateScores Blue %d / Red %d"), BlueScore, RedScore);
}
