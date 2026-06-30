// Fill out your copyright notice in the Description page of Project Settings.


#include "SGPlayerSlotWidget.h"

#include "Components/Border.h"
#include "Components/TextBlock.h"

void USGPlayerSlotWidget::NativeConstruct()
{
	Super::NativeConstruct();
	
	// TODO: 기본 이름 텍스트 값 -> 일단은 정해진 값 할당, 나중에 랜덤 할당
	if (Text_UserName)
	{
		Text_UserName->SetText(FText::FromString(TEXT("Player1")));	
	}
	
	
	// TODO: 기본 Ready 표시 상태
	if (Text_Ready)
	{
		Text_Ready->SetText(FText::FromString(TEXT("Ready")));	
	}
	
	// TODO: 기본 배경색
	if (Border_Background)
	{
		Border_Background->SetBrushColor(FLinearColor::Blue);	
	}
	
}

void USGPlayerSlotWidget::SetPlayerSlotInfo(const FText& InUserName, bool bInReady, ESGTeamType InTeamType)
{
	// TODO: Text_UserName 유효성 검사
	// TODO: Text_UserName에 InUserName 반영
	if (!Text_UserName) return;
	Text_UserName->SetText(InUserName);
	
	
	// TODO: Text_Ready 유효성 검사
	// TODO: bInReady 값에 따른 Test_Ready 표시 상태 반영
	if (!Text_Ready) return; 
	if (bInReady)
	{
		Text_Ready->SetText(FText::FromString(TEXT("Ready!")));
		Text_Ready->SetVisibility(ESlateVisibility::Visible);
	}
	else
	{
		Text_Ready->SetText(FText::GetEmpty());
		Text_Ready->SetVisibility(ESlateVisibility::Hidden);
	}
	
	// TODO: Border_Root 유효성 검사
	
	// TODO: InTeamType에 따른 Border_Root 배경색 반영
	if (Border_Background)
	{
		Border_Background->SetBrushColor(GetTeamColor(InTeamType));
	}
}

FLinearColor USGPlayerSlotWidget::GetTeamColor(ESGTeamType InTeamType) const
{
	if (InTeamType == ESGTeamType::Blue)
	{
		return FLinearColor::Blue;
	}
	else if (InTeamType == ESGTeamType::Red)
	{
		return FLinearColor::Red;
	}
	else if (InTeamType == ESGTeamType::Waiting)
	{
		return FLinearColor::Gray;
	}
	
	return FLinearColor::Gray;
}




