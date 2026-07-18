// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/SGPlayerSlotWidget.h"
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
	
	// if (Border_Background)
	// {
	// 	Border_Background->SetBrushColor(GetTeamColor(MySlotTeamTag));
	// }
	UpdateButtonStyle(MySlotTeamTag, 0.3f);
}

void USGPlayerSlotWidget::ResetSlot()
{
	if (!Text_UserName)
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
	
	if (MySlotTeamTag.IsValid())
	{
		UpdateButtonStyle(MySlotTeamTag,0.3f);
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
	// if (Border_Background)
	// {
	// 	Border_Background->SetBrushColor(GetTeamColor(InTeamTag));
	// 	Border_Background->SetRenderOpacity(1.0f);
	// }
	
	UpdateButtonStyle(InTeamTag, 1.0f);
}

void USGPlayerSlotWidget::UpdateButtonStyle(FGameplayTag InTeamTag, float InOpacity)
{
	if (!Button_SlotClick) return;
    
	FButtonStyle NewStyle = Button_SlotClick->GetStyle();
    
	FLinearColor NormalColor;
	FLinearColor HoverColor;
	FLinearColor PressedColor;
    
	FGameplayTag BlueTag = FGameplayTag::RequestGameplayTag(FName("Team.Blue"));
	FGameplayTag RedTag = FGameplayTag::RequestGameplayTag(FName("Team.Red"));
    
	// 에디터에서 설정한 색상 가져오기
	if (InTeamTag == BlueTag)
	{
		NormalColor  = BlueColor_Normal;
		HoverColor   = BlueColor_Hovered;
		PressedColor = BlueColor_Pressed;
	}
	else if (InTeamTag == RedTag)
	{
		NormalColor  = RedColor_Normal;
		HoverColor   = RedColor_Hovered;
		PressedColor = RedColor_Pressed;
	}
	else
	{
		NormalColor  = WaitingColor_Normal;
		HoverColor   = WaitingColor_Hovered;
		PressedColor = WaitingColor_Pressed;
	}
    
	// 알파(투명도) 값에 InOpacity를 곱해서 빈 슬롯(0.3) / 찬 슬롯(1.0) 효과 적용
	NormalColor.A *= InOpacity;
	HoverColor.A *= InOpacity;
	PressedColor.A *= InOpacity;
    
	// 스타일 적용
	NewStyle.Normal.TintColor = FSlateColor(NormalColor);
	NewStyle.Hovered.TintColor = FSlateColor(HoverColor);
	NewStyle.Pressed.TintColor = FSlateColor(PressedColor);
    
	Button_SlotClick->SetStyle(NewStyle);
}

void USGPlayerSlotWidget::OnButtonClicked()
{	
	OnSlotClicked.Broadcast(MySlotTeamTag);
}




