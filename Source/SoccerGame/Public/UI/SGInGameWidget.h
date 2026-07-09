// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "SGInGameWidget.generated.h"

// 전방선언
struct FOnAttributeChangeData;
class USGItemSlotComponent;
class UImage;
class UTextBlock;
class USGScoreBoardWidget;
class UTexture2D;
class WBP_ItemSlot;
/**
 * 
 */
UCLASS()
class SOCCERGAME_API USGInGameWidget : public UUserWidget
{
	GENERATED_BODY()
	
public:
	virtual void NativeConstruct() override;
	
	// 외부에 노출할 UI 갱신 함수 (전광판 조작 버튼)
	void UpdateTimerUI(int32 NewTime);
	void UpdateScores(int32 BlueScore , int32 RedScore);
	
protected:
	virtual void NativeTick(const FGeometry& MyGeometry, float InDeltaTime) override;
	
	virtual void NativeDestruct() override;
	
	
	UFUNCTION()
	void RefreshAllItemSlots();

	
	

protected:
	UPROPERTY()
	TObjectPtr<USGItemSlotComponent> ItemSlotComp;
	
	// // TODO: WBP_InGame에서 바인딩할 위젯 2개
	// UPROPERTY(BlueprintReadWrite, meta = (BindWidget))
	// TObjectPtr<UUserWidget> WBP_ItemSlot_0;
	//
	// UPROPERTY(BlueprintReadWrite, meta = (BindWidget))
	// TObjectPtr<UUserWidget> WBP_ItemSlot_1;
	
	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UUserWidget> WBP_Timer;
	
	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<USGScoreBoardWidget> WBP_ScoreBoard;
	
private:
	// 이전 프레임의 점수 저장
	int32 LastBlueTeamScore = -1;
	int32 LastRedTeamScore = -1;
	
#pragma region Stamina
public:
	// 스태미나 델리게이트 수신용 콜백 함수
	void GAS_OnStaminaChanged(const FOnAttributeChangeData& Data);
	
	
protected:
	// 시각적 표현을 위해 블루프린트로 값 넘겨주는 이벤트
	UFUNCTION(BlueprintImplementableEvent, Category = "UI|Stamina")
	void BP_UpdateStaminaUI(float StaminaPercent);
	
	UFUNCTION(BlueprintImplementableEvent, Category = "UI|Item")
	void BP_UpdateItemSlotUI(int32 SlotIndex, UTexture2D* Icon);
	
	
#pragma endregion
};
