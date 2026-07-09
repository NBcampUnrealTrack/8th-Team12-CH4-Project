// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/SGInGameWidget.h"
#include "UI/SGScoreBoardWidget.h"
#include "Components/TextBlock.h"
#include "SoccerGame/Public/Item/SGItemSlotComponent.h"
#include "SoccerGame/Public/Item/Data/SGItemDefinition.h"
#include "Components/Image.h"
#include "GameState/SGMainGameState.h"
#include "Kismet/GameplayStatics.h"

void USGInGameWidget::NativeConstruct()
{
	Super::NativeConstruct();
	APawn* MyPawn = GetOwningPlayerPawn();
	if (MyPawn)
	{
		ItemSlotComp = MyPawn->FindComponentByClass<USGItemSlotComponent>();
		if (ItemSlotComp)
		{
			ItemSlotComp->OnItemSlotChanged.AddDynamic(this, &USGInGameWidget::RefreshAllItemSlots);
			RefreshAllItemSlots();
		}
	}
}

void USGInGameWidget::UpdateTimerUI(int32 NewTime)
{
	if (!IsValid(WBP_Timer)) return;
	UTextBlock* RealTimerTextBlock = Cast<UTextBlock>(WBP_Timer->GetWidgetFromName(TEXT("Text_Timer")));

	// 초 단위 시간을 MM:SS 형식으로 변환
	int32 Minutes = NewTime / 60;
	int32 Seconds = NewTime % 60;

	// FString::Printf를 사용하여 두 자리 숫자로 패딩 ("%02d")
	FString TimeString = FString::Printf(TEXT("%02d:%02d"), Minutes, Seconds);
	
	RealTimerTextBlock->SetText(FText::FromString(TimeString));
	//WBP_Timer->SetText(FText::FromString(TimeString));
}

void USGInGameWidget::UpdateScores(int32 BlueScore, int32 RedScore)
{
	// 점수 변경이 발생했을 때만 업데이트 (불필요한 호출 방지)
	if (LastRedTeamScore != RedScore || LastBlueTeamScore != BlueScore)
	{
		LastRedTeamScore = RedScore;
		LastBlueTeamScore = BlueScore;
		if (IsValid(WBP_ScoreBoard))
		{
			WBP_ScoreBoard->UpdateScores( BlueScore,RedScore);
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

void USGInGameWidget::RefreshAllItemSlots()
{
	//UpdateSingleItemSlot(0, WBP_ItemSlot);
	//UpdateSingleItemSlot(1, WBP_ItemSlot_1);
}

void USGInGameWidget::UpdateSingleItemSlot(int32 Index, UImage* TargetImage)
{
	if (!ItemSlotComp || !TargetImage)
	{
		// 디버그 메세지
		UE_LOG(LogTemp, Warning, TEXT("ItemSlotComp 혹은 TargetImage가 유효하지 않습니다."));
		return;
	}
	
	USGItemDefinition* ItemDef = ItemSlotComp->GetItemAt(Index);

	if (ItemDef&& ItemDef->Icon)
	{
		TargetImage->SetBrushFromTexture(ItemDef->Icon);
		TargetImage->SetVisibility(ESlateVisibility::Visible);
	}
	else
	{
		TargetImage->SetVisibility(ESlateVisibility::Collapsed);
	}
}

void USGInGameWidget::NativeDestruct()
{
	if (ItemSlotComp)
	{
		ItemSlotComp->OnItemSlotChanged.RemoveDynamic(this, &USGInGameWidget::RefreshAllItemSlots);
	}
	Super::NativeDestruct();
}

