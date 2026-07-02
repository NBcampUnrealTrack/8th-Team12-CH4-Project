// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "GameplayTagContainer.h"
#include "SGLobbyWidget.generated.h"

class UVerticalBox;
class USGPlayerSlotWidget;
class UButton;
class UTextBlock;

USTRUCT(BlueprintType)
struct FSGPlayerLobbyInfo
{
	GENERATED_BODY()
	
	// 플레이어 이름 변수 선언
	UPROPERTY(BlueprintReadWrite)
	FText UserName;
	
	// Ready 상태 변수 선언
	UPROPERTY(BlueprintReadWrite)
	bool bIsReady = false;
	
	// 팀 타입 변수 선언
	UPROPERTY(BlueprintReadWrite)
	FGameplayTag TeamTag;
};

/**
 * 
 */
UCLASS()
class SOCCERGAME_API USGLobbyWidget : public UUserWidget
{
	GENERATED_BODY()
	
protected:
	virtual void NativeConstruct() override;
	
#pragma region PlayerSlots properties
public:
	// 블루 팀 슬롯 3개 바인딩
	UPROPERTY(meta =(BindWidget))
	class USGPlayerSlotWidget* BlueSlot_1;
	UPROPERTY(meta =(BindWidget))
	class USGPlayerSlotWidget* BlueSlot_2;
	UPROPERTY(meta =(BindWidget))
	class USGPlayerSlotWidget* BlueSlot_3;
	
	// 레드 팀 슬롯 3개 바인딩
	UPROPERTY(meta =(BindWidget))
	class USGPlayerSlotWidget* RedSlot_1;
	UPROPERTY(meta =(BindWidget))
	class USGPlayerSlotWidget* RedSlot_2;
	UPROPERTY(meta =(BindWidget))
	class USGPlayerSlotWidget* RedSlot_3;
	
	// 대기 슬롯 6개 바인딩
	UPROPERTY(meta = (BindWidget))
	class USGPlayerSlotWidget* WaitingSlot_1;
	UPROPERTY(meta = (BindWidget))
	class USGPlayerSlotWidget* WaitingSlot_2;
	UPROPERTY(meta = (BindWidget))
	class USGPlayerSlotWidget* WaitingSlot_3;
	UPROPERTY(meta = (BindWidget))
	class USGPlayerSlotWidget* WaitingSlot_4;
	UPROPERTY(meta = (BindWidget))
	class USGPlayerSlotWidget* WaitingSlot_5;
	UPROPERTY(meta = (BindWidget))
	class USGPlayerSlotWidget* WaitingSlot_6;
	
protected:
	UPROPERTY()
	TArray<USGPlayerSlotWidget*> BlueTeamSlots;
	UPROPERTY()
	TArray<USGPlayerSlotWidget*> RedTeamSlots;
	UPROPERTY()
	TArray<USGPlayerSlotWidget*> WaitingSlots;
	
	// Ready 버튼 바인딩 변수
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UButton> ReadyButton;
	
	// Ready 버튼 텍스트 바인딩 변수
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> Text_ReadyButton;
	
	UPROPERTY()
	int32 LocalPlayerIndex = 0;
	
	// PlayerSlot 위젯 클래스 변수
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Lobby")
	TSubclassOf<USGPlayerSlotWidget> PlayerSlotWidgetClass;
	
	// Player 정보 목록 배열
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Lobby")
	TArray<FSGPlayerLobbyInfo> PlayerInfos;
#pragma endregion
	
public:
	// 로비 UI 갱신 함수 시그니처
	UFUNCTION(BlueprintCallable)
	void RefreshLobby();

	// 플레이어 목록 세팅 함수 시그니처
	UFUNCTION(BlueprintCallable)
	void SetPlayerInfos(const TArray<FSGPlayerLobbyInfo>& InPlayerInfos);
	
	// Ready 버튼 클릭 콜백 함수 시그니처
	UFUNCTION()
	void OnReadyButtonClicked();
	
	// Ready 버튼 텍스트 갱신 함수 시그니처
	void UpdateReadyButtonText();
	
protected:
	UFUNCTION()
	void HandleSlotClicked(FGameplayTag RequestedTeamTag);
	
	
};
