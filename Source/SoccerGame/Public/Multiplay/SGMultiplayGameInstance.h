// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstance.h"
#include "Interfaces/OnlineSessionInterface.h"
#include "SGMultiplayGameInstance.generated.h"

/**
 * 
 */
UCLASS()
class SOCCERGAME_API USGMultiplayGameInstance : public UGameInstance
{
	GENERATED_BODY()
	
public:
	USGMultiplayGameInstance();

	UFUNCTION(BlueprintCallable, Category = "MultiPlayer")
	void CreateServer();
	
protected:
	virtual void Init() override;
	
	// 방 생성 완료시 실행될 함수
	void OnCreateSessionComplete(FName Sessionname, bool bWasSuccessful);
	
private:
	// 엔진의 세션 기능을 담당할 세션 매니저 포인터
	IOnlineSessionPtr SessionInterface;
	
	// 델리게이트 관련 변수
	FOnCreateSessionCompleteDelegate CreateSessionCompleteDelegate;
	FDelegateHandle CreateSessionCompleteDelegateHandle;
};
