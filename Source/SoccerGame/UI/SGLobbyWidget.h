// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "SGPlayerSlotWidget.h"
#include "Blueprint/UserWidget.h"
#include "SGLobbyWidget.generated.h"

class UVerticalBox;
class USGPlayerSlotWidget;
class UButton;
class UTextBlock;

USTRUCT(BlueprintType)
struct FSGPlayerLobbyInfo
{
	GENERATED_BODY()
	
	// TODO: 플레이어 이름 변수 선언
	UPROPERTY(BlueprintReadWrite)
	FText UserName;
	
	// TODO: Ready 상태 변수 선언
	UPROPERTY(BlueprintReadWrite)
	bool bIsReady = false;
	
	// TODO: 팀 타입 변수 선언
	UPROPERTY(BlueprintReadWrite)
	ESGTeamType TeamType = ESGTeamType::Waiting;
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
	
protected:
	// TODO: Blue 팀 VerticalBox 바인딩 변수
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UVerticalBox> VerticalBox_BlueTeam;
	
	// TODO: Red 팀 VerticalBox 바인딩 변수
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UVerticalBox> VerticalBox_RedTeam;
	
	// TODO: Red 팀 VerticalBox 바인딩 변수
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UVerticalBox> VerticalBox_Waiting;
	
	// TODO: Ready 버튼 바인딩 변수
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UButton> ReadyButton;
	
	// TODO: Ready 버튼 텍스트 바인딩 변수
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> Text_ReadyButton;
	
	UPROPERTY()
	int32 LocalPlayerIndex = 0;
	
	// TODO: PlayerSlot 위젯 클래스 변수
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Lobby")
	TSubclassOf<USGPlayerSlotWidget> PlayerSlotWidgetClass;
	
	// TODO: Ready 버튼 텍스트 갱신 함수 시그니처
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Lobby")
	TArray<FSGPlayerLobbyInfo> PlayerInfos;

public:
	// TODO: 로비 UI 갱신 함수 시그니처
	UFUNCTION(BlueprintCallable)
	void RefreshLobby();
	
	// TODO: 플레이어 목록 세팅 함수 시그니처
	UFUNCTION(BlueprintCallable)
	void SetPlayerInfos(const TArray<FSGPlayerLobbyInfo>& InPlayerInfos);

	// TODO: Ready 버튼 클릭 콜백 함수 시그니처
	UFUNCTION()
	void OnReadyButtonClicked();
	
	// TODO: Ready 버튼 텍스트 갱신 함수 시그니처
	void UpdateReadyButtonText();
	
	
};
