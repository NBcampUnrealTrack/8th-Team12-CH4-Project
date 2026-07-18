// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "GameplayTagContainer.h"
#include "SGPlayerSlotWidget.generated.h"

class UTextBlock;
class UBorder;
class USGLobbyWidget;
class UButton;
/**
 * 
 */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnSlotClickedSignature, FGameplayTag, ClickedTeamType);

UCLASS()
class SOCCERGAME_API USGPlayerSlotWidget : public UUserWidget
{
	GENERATED_BODY()
	
public:
	
	// 위젯 생성 시 초기화 함수 오버라이드
	virtual auto NativeConstruct() -> void override;
	
	UPROPERTY(BlueprintAssignable, Category = "Events")
	FOnSlotClickedSignature OnSlotClicked;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Team")
	FGameplayTag MySlotTeamTag;
	
	void SetSlotTeamTag(FGameplayTag InTeamTag);
	
	UFUNCTION(BlueprintCallable)
	void ResetSlot();
	
	// 플레이어 슬롯 정보 외부에서 주입받기
	UFUNCTION(BlueprintCallable)
	void SetPlayerSlotInfo(const FString& InUserName, bool bInReady, FGameplayTag InTeamType);

protected:
	// 플레이어 이름 TextBlock 바인딩 변수
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> Text_UserName;
	
	// Ready 표시 TestBlock 바인딩 변수
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> Text_Ready;
	
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UButton> Button_SlotClick;

	void UpdateButtonStyle(FGameplayTag InTeamTag, float InOpacity);
	
private:
	
	
	// // 버튼 스타일
	// UPROPERTY(EditAnywhere, Category = "Style")
	// FButtonStyle BlueButtonStyle;
	//
	// UPROPERTY(EditAnywhere, Category = "Style")
	// FButtonStyle RedButtonStyle;
	
	UFUNCTION()
	void OnButtonClicked();

public:
	UPROPERTY(EditDefaultsOnly, Category = "UI|SlotColors|BlueTeam")
	FLinearColor BlueColor_Normal = FLinearColor(0.1f, 0.3f, 0.8f, 1.0f);
    
	UPROPERTY(EditDefaultsOnly, Category = "UI|SlotColors|BlueTeam")
	FLinearColor BlueColor_Hovered = FLinearColor(0.3f, 0.5f, 1.0f, 1.0f);
    
	UPROPERTY(EditDefaultsOnly, Category = "UI|SlotColors|BlueTeam")
	FLinearColor BlueColor_Pressed = FLinearColor(0.05f, 0.15f, 0.5f, 1.0f);

	// --- 레드팀 색상 설정 ---
	UPROPERTY(EditDefaultsOnly, Category = "UI|SlotColors|RedTeam")
	FLinearColor RedColor_Normal = FLinearColor(0.8f, 0.2f, 0.2f, 1.0f);
    
	UPROPERTY(EditDefaultsOnly, Category = "UI|SlotColors|RedTeam")
	FLinearColor RedColor_Hovered = FLinearColor(1.0f, 0.4f, 0.4f, 1.0f);
    
	UPROPERTY(EditDefaultsOnly, Category = "UI|SlotColors|RedTeam")
	FLinearColor RedColor_Pressed = FLinearColor(0.5f, 0.1f, 0.1f, 1.0f);

	// --- 대기열(Waiting) 색상 설정 ---
	UPROPERTY(EditDefaultsOnly, Category = "UI|SlotColors|Waiting")
	FLinearColor WaitingColor_Normal = FLinearColor(0.5f, 0.5f, 0.5f, 1.0f);
    
	UPROPERTY(EditDefaultsOnly, Category = "UI|SlotColors|Waiting")
	FLinearColor WaitingColor_Hovered = FLinearColor(0.7f, 0.7f, 0.7f, 1.0f);
    
	UPROPERTY(EditDefaultsOnly, Category = "UI|SlotColors|Waiting")
	FLinearColor WaitingColor_Pressed = FLinearColor(0.3f, 0.3f, 0.3f, 1.0f);
};
