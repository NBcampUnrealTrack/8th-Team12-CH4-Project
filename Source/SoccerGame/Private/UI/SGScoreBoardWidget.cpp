// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/SGScoreBoardWidget.h"
#include "Components/TextBlock.h"

void USGScoreBoardWidget::UpdateScores(int32 BlueScore, int32 RedScore)
{
	if (Text_BlueScore)
	{
		Text_BlueScore->SetText(FText::AsNumber(BlueScore));
	}
	if (Text_RedScore)
	{
		Text_RedScore->SetText(FText::AsNumber(RedScore));
	}
}
