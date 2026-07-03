// Fill out your copyright notice in the Description page of Project Settings.


#include "SGInGameWidget.h"
#include "Components/TextBlock.h"


void USGInGameWidget::NativeConstruct()
{
	Super::NativeConstruct();
	
	// 타이머 0으로 세팅
	UpdateTimerText(0);
}

void USGInGameWidget::UpdateTimerText(int32 CurrentTime)
{
	
	if (!Text_Timer) return;
	
	int32 Minutes = CurrentTime / 60;
	int32 Seconds = CurrentTime % 60;
	
	// 시간 포맷팅
	// 무조건 00:00 두자리씩 표시
	FString TimeString = FString::Printf(TEXT("%d:%02d"), Minutes, Seconds);
	
	Text_Timer->SetText(FText::FromString(TimeString));
}

