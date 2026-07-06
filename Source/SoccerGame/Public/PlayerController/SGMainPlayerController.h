// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "SGMainPlayerController.generated.h"


class USGInGameWidget;
UCLASS()
class SOCCERGAME_API ASGMainPlayerController : public APlayerController
{
	GENERATED_BODY()
public:
	UFUNCTION(Client, Reliable)
	void UpdateTimerWidget(int32 NewTime);
protected:
	virtual void BeginPlay() override;
	virtual void OnPossess(APawn* InPawn) override;
	virtual void AcknowledgePossession(APawn* P) override;

private:
	void ApplyGameInputMode();
	
protected:
	// 주기적으로 로드를 시도할 타이머 핸들
	FTimerHandle LoadDataTimerHandle;

	// 실제 데이터를 로드하고 성공 여부를 반환하는 함수
	void LoadPlayerData();
	// 에디터에서 할당할 위젯 클래스 (TSubclassOf 선언 시 실제 위젯 타입 지정)
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "UI")
	TSubclassOf<USGInGameWidget> UIMainGameWidgetClass;
	
	// 생성된 위젯 인스턴스를 안전하게 보관할 멤버 변수
	UPROPERTY()
	USGInGameWidget* UIMainGameWidgetInstance;
};
