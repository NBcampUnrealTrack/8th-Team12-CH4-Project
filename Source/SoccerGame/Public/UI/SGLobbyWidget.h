// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "GameplayTagContainer.h"
#include "Character/SGCharacterDataAsset.h"
#include "SGLobbyWidget.generated.h"

class UVerticalBox;
class USGPlayerSlotWidget;
class UButton;
class UTextBlock;
class UImage;
class USGCharacterDataAsset;

USTRUCT(BlueprintType)
struct FSGPlayerLobbyInfo
{
	GENERATED_BODY()

public:
	// 플레이어 이름 변수 선언
	UPROPERTY(BlueprintReadWrite)
	FString UserName;
	
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
	virtual void NativeDestruct() override;

	
#pragma region Binding
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
	
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UButton> Button_BackToMenu;
	
	// Ready 후 Start Timer
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> Text_StartTimer;
	
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UButton> Button_ChangeUserName;

	
	
#pragma endregion
	
#pragma region Player
protected:
	// PlayerSlot 위젯 클래스 변수
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Lobby")
	TSubclassOf<USGPlayerSlotWidget> PlayerSlotWidgetClass;
	
	// Player 정보 목록 배열
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Lobby")
	TArray<FSGPlayerLobbyInfo> PlayerInfos;
	
#pragma endregion
	
public:
	
	void AddPlayerInfos(const FSGPlayerLobbyInfo& InPlayerInfos)
	{ PlayerInfos.Add(InPlayerInfos); }
	
	// 로비 UI 갱신 함수 시그니처
	UFUNCTION(BlueprintCallable)
	void RefreshLobby();

	// 플레이어 목록 세팅 함수 시그니처
	UFUNCTION(BlueprintCallable)
	void SetPlayerInfos(const TArray<FSGPlayerLobbyInfo>& InPlayerInfos);
	
	void UpdateCountdownText(int32 NewTime);
	

	
protected:

	// 슬롯 클릭되었을 때 실행될 함수
	UFUNCTION()
	void HandleSlotClicked(FGameplayTag RequestedTeamTag);

#pragma region ReadyButton
	
protected:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "UI|ReadyButton")
	UTexture2D* Image_Ready_Normal;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "UI|ReadyButton")
	UTexture2D* Image_Ready_Hover;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "UI|ReadyButton")
	UTexture2D* Image_Ready_Pressed;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "UI|ReadyButton")
	UTexture2D* Image_Cancel_Normal;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "UI|ReadyButton")
	UTexture2D* Image_Cancel_Hover;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "UI|ReadyButton")
	UTexture2D* Image_Cancel_Pressed;
	
public:
	// Ready 버튼 클릭 콜백 함수 시그니처
	UFUNCTION()
	void OnReadyButtonClicked();
	
	// Ready 버튼 텍스트 갱신 함수 시그니처
	void UpdateReadyButtonText();
	
#pragma endregion
	
#pragma region Select Character
	
protected:
	UPROPERTY(meta = (BindWidget))
	UImage* Image_Character;
	
	UPROPERTY(meta = (BindWidget))
	UButton* Button_Left;
	
	UPROPERTY(meta = (BindWidget))
	UButton* Button_Right;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Character")
	TArray<USGCharacterDataAsset*> CharacterList;
	
	int32 CurrentIndex = 0;
	
	UFUNCTION()
	TArray<USGCharacterDataAsset*> GetFilteredCharacterList();
	
public:
	// 캐릭터 변경 버튼 클릭 함수
	UFUNCTION(BlueprintCallable, Category = "Character")
	void OnNextButtonClicked();

	UFUNCTION(BlueprintCallable, Category = "Character")
	void OnPrevButtonClicked();
	
	UFUNCTION(BlueprintCallable)
	void RefreshCharacterSelection();
	
#pragma endregion

#pragma region Sound
public:
	UPROPERTY(EditDefaultsOnly, Category = "UI|Sounds")
	class USoundBase* Sound_TimerTick;
#pragma endregion

private:
	
	UFUNCTION()
	void OnClickedBackToMenuButton();
	
	UFUNCTION()
	void OnClickedBackMainMenuButton();
	
	UFUNCTION()
	void OnClickedChangeUsernameButton();
	

	
	
};
