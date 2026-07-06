// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/SGPlayerSlotWidget.h"
#include "Components/Border.h"
#include "Components/Button.h"
#include "Components/TextBlock.h"

void USGPlayerSlotWidget::NativeConstruct()
{
	Super::NativeConstruct();
	
	if (Button_SlotClick)
	{
		Button_SlotClick->OnClicked.AddDynamic(this, &USGPlayerSlotWidget::OnButtonClicked);
	}
	
	ResetSlot();
}

void USGPlayerSlotWidget::SetSlotTeamTag(FGameplayTag InTeamTag)
{
	MySlotTeamTag = InTeamTag;
	
	if (Border_Background)
	{
		Border_Background->SetBrushColor(GetTeamColor(MySlotTeamTag));
	}
}

void USGPlayerSlotWidget::ResetSlot()
{
	if (!Text_UserName || !Border_Background)
	{
		UE_LOG(LogTemp, Error, TEXT("치명적 에러: WBP_PlayerSlot 내부 부품이 연결 안 됨! 변수(Is Variable) 체크를 확인하세요!"));
	}
	
	if (Text_UserName)
	{
		Text_UserName->SetVisibility(ESlateVisibility::Hidden);
	}
	if (Text_Ready)
	{
		Text_Ready->SetVisibility(ESlateVisibility::Collapsed);
		Text_Ready->SetText(FText::GetEmpty());
	}
	if (Border_Background)
	{
		Border_Background->SetRenderOpacity(0.3f);
		
		if (MySlotTeamTag.IsValid())
		{
			Border_Background->SetBrushColor(GetTeamColor(MySlotTeamTag));
		}
	}
}

void USGPlayerSlotWidget::SetPlayerSlotInfo(const FString& InUserName, bool bInReady, FGameplayTag InTeamTag)
{
	
	// Text_UserName 유효성 검사
	// Text_UserName에 InUserName 반영
	if (Text_UserName)
	{
		Text_UserName->SetText(FText::FromString(InUserName));
		Text_UserName->SetVisibility(ESlateVisibility::SelfHitTestInvisible);
		
		FGameplayTag WaitingTag = FGameplayTag::RequestGameplayTag(FName("Team.Waiting"));
		
		if (InTeamTag == WaitingTag)
		{
			Text_UserName->SetJustification(ETextJustify::Center);
			
			if (Text_Ready)
			{
				Text_Ready->SetVisibility(ESlateVisibility::Collapsed);
			}
		}
		else
		{
			Text_UserName->SetJustification(ETextJustify::Left);
			
			if (Text_Ready)
			{
				Text_Ready->SetVisibility(ESlateVisibility::SelfHitTestInvisible);
				
				if (bInReady)
				{
					Text_Ready->SetText(FText::FromString(TEXT("Ready!")));
				}
				else
				{
					Text_Ready->SetText(FText::GetEmpty());
				}
			}
		}
		
		
	}
	
	// InTeamTag에 따른 Border_Background 배경색 반영 및 투명도 원상 복구
	if (Border_Background)
	{
		Border_Background->SetBrushColor(GetTeamColor(InTeamTag));
		Border_Background->SetRenderOpacity(1.0f);
	}
}

FLinearColor USGPlayerSlotWidget::GetTeamColor(FGameplayTag InTeamTag) const
{
	if (InTeamTag == FGameplayTag::RequestGameplayTag(FName("Team.Blue")))
	{
		return FLinearColor::Blue;
	}
	else if (InTeamTag == FGameplayTag::RequestGameplayTag(FName("Team.Red")))
	{
		return FLinearColor::Red;
	}
	
	return FLinearColor::Gray;
}

void USGPlayerSlotWidget::OnButtonClicked()
{
	OnSlotClicked.Broadcast(MySlotTeamTag);
}




