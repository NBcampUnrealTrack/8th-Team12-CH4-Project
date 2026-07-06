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
protected:
	virtual void BeginPlay() override;
	virtual void OnPossess(APawn* InPawn) override;
	virtual void AcknowledgePossession(APawn* P) override;

private:
	void ApplyGameInputMode();
	
protected:
	// 에디터에서 할당할 위젯 클래스 (TSubclassOf 선언 시 실제 위젯 타입 지정)
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "UI")
	TSubclassOf<USGInGameWidget> UIMainGameWidgetClass;
	
	// 생성된 위젯 인스턴스를 안전하게 보관할 멤버 변수
	UPROPERTY()
	USGInGameWidget* UIMainGameWidgetInstance;
};
