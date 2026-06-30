// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "SGPlayerSlotWidget.generated.h"

class UTextBlock;
class UBorder;
/**
 * 
 */

/* 팀 상태 Enum */
UENUM(BlueprintType)
enum class ESGTeamType : uint8
{
	Blue,
	Red,
	Waiting
};

UCLASS()
class SOCCERGAME_API USGPlayerSlotWidget : public UUserWidget
{
	GENERATED_BODY()
	
public:
	// TODO: 플레이어 슬롯 정보 외부에서 주입받기
	UFUNCTION(BlueprintCallable)
	void SetPlayerSlotInfo(const FText& InUserName, bool bInReady, ESGTeamType InTeamType);
protected:
	// TODO: 위젯 생성 시 초기화 함수 오버라이드
	virtual void NativeConstruct() override;
	
protected:
	// TODO: 플레이어 이름 TextBlock 바인딩 변수
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> Text_UserName;
	
	// TODO: Ready 표시 TestBlock 바인딩 변수
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> Text_Ready;
	
	// TODO: 슬롯 배경 Border 바인딩 변수
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UBorder> Border_Background;
	
private:
	// TODO: 팀 타입에 따른 배경색 반환 함수 시그니처
	FLinearColor GetTeamColor(ESGTeamType InTeamType) const;
};
