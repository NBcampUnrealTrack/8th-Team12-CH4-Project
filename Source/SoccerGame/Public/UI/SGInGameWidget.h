// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "SGInGameWidget.generated.h"

class UTextBlock;
class USGScoreBoardWidget;
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
	void UpdateScores(int32 RedScore, int32 BlueScore);
	
protected:
	virtual void NativeTick(const FGeometry& MyGeometry, float InDeltaTime) override;
	
protected:
	// 타이머 텍스트
	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UTextBlock> Text_Timer;
	
	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<USGScoreBoardWidget> WBP_ScoreBoardWidget;
	
private:
	// 이전 프레임의 점수 저장
	int32 LastBlueTeamScore = -1;
	int32 LastRedTeamScore = -1;
};
