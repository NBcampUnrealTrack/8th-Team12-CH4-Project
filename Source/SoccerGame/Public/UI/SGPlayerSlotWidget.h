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
	
	// 슬롯 배경 Border 바인딩 변수
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UBorder> Border_Background;
	
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UButton> Button_SlotClick;

	
private:
	// 팀 타입에 따른 배경색 반환 함수 시그니처
	FLinearColor GetTeamColor(FGameplayTag InTeamTag) const;
	
	// 버튼 스타일
	UPROPERTY(EditAnywhere, Category = "Style")
	FButtonStyle BlueButtonStyle;
	
	UPROPERTY(EditAnywhere, Category = "Style")
	FButtonStyle RedButtonStyle;
	
	UFUNCTION()
	void OnButtonClicked();
};
