// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/SGInGameWidget.h"
#include "UI/SGScoreBoardWidget.h"
#include "Components/TextBlock.h"
#include "SoccerGame/Public/Item/SGItemSlotComponent.h"
#include "SoccerGame/Public/Item/Data/SGItemDefinition.h"
#include "Components/Image.h"
#include "AbilitySystemGlobals.h"
#include "AbilitySystemComponent.h"
#include "Character/GAS/GAS_SG_CharacterAttributeSet.h"

void USGInGameWidget::NativeConstruct()
{
	Super::NativeConstruct();
	
	APawn* MyPawn = GetOwningPlayerPawn();
	UAbilitySystemComponent* ASC = UAbilitySystemGlobals::GetAbilitySystemComponentFromActor(MyPawn);
	if (MyPawn)
	{
		// 스태미나 Attribute 연결
		
		if (ASC)
		{
			ASC->GetGameplayAttributeValueChangeDelegate(UGAS_SG_CharacterAttributeSet::GetStaminaAttribute())
				.AddUObject(this, &USGInGameWidget::GAS_OnStaminaChanged);
		}
		
		
		// 아이템 슬롯 컴포넌트 연결
		ItemSlotComp = MyPawn->FindComponentByClass<USGItemSlotComponent>();
		if (ItemSlotComp)
		{
			ItemSlotComp->OnItemSlotChanged.AddDynamic(this, &USGInGameWidget::RefreshAllItemSlots);
			RefreshAllItemSlots();
		}
		else
		{
			// 아이템 슬롯 컴포넌트가 없을 때
			UE_LOG(LogTemp, Error, TEXT("[UI 에러] 캐릭터에서 ItemSlotComponent를 찾을 수 없습니다."));
		}
	}
	else
	{
		// 	UI가 너무 빨리 켜져서 캐릭터를 못 찾았을 때
		UE_LOG(LogTemp, Error, TEXT("[UI 에러] MyPawn이 Null입니다! (UI 생성 타이밍 문제)"));
	}
	
	// 킥파워 변화 감지 바인딩
	
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

void USGInGameWidget::NativeDestruct()
{
	if (ItemSlotComp)
	{
		ItemSlotComp->OnItemSlotChanged.RemoveDynamic(this, &USGInGameWidget::RefreshAllItemSlots);
	}
	Super::NativeDestruct();
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

void USGInGameWidget::UpdateScores(int32 RedScore, int32 BlueScore)
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

void USGInGameWidget::RefreshAllItemSlots()
{
	if (!ItemSlotComp) return;
	
	// TODO: 아이템 슬롯 개수 ItemSlotComponent->MaxItemCount 가져오는걸로 바꾸기
	// 아이템 아이콘 가져와서 업데이트
	for (int32 i = 0; i < 2; ++i)
	{
		USGItemDefinition* ItemDef = ItemSlotComp->GetItemAt(i);
		UTexture2D* Icon = (ItemDef != nullptr) ? ItemDef->Icon : nullptr;
		
		BP_UpdateItemSlotUI(i, Icon);
	}
}

void USGInGameWidget::GAS_OnStaminaChanged(const FOnAttributeChangeData& Data)
{
	float CurrentStamina = Data.NewValue;
	
	UAbilitySystemComponent* ASC = UAbilitySystemGlobals::GetAbilitySystemComponentFromActor(GetOwningPlayerPawn());
	if (!ASC) return;
	
	float MaxStamina = ASC->GetNumericAttribute(UGAS_SG_CharacterAttributeSet::GetMaxStaminaAttribute());
	// 0으로 나누기 방지
	float StaminaPercent = (MaxStamina > 0.f) ? (CurrentStamina / MaxStamina) : (0.f);
	
	BP_UpdateStaminaUI(StaminaPercent);
}


