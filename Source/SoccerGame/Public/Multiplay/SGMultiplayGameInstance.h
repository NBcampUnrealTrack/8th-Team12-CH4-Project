// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstance.h"
#include "Interfaces/OnlineSessionInterface.h"
#include "OnlineSessionSettings.h"
#include "SGMultiplayGameInstance.generated.h"

class FOnlineSessionSearch;
class USGCharacterDataAsset;
/**
 * 
 */
UCLASS()
class SOCCERGAME_API USGMultiplayGameInstance : public UGameInstance
{
	GENERATED_BODY()
	
public:
	
	// 델리게이트 및 핸들 선언 추가 
	FOnDestroySessionCompleteDelegate DestroySessionCompleteDelegate;
	FDelegateHandle DestroySessionCompleteDelegateHandle;

	// 콜백 함수 선언 추가
	void OnDestroySessionComplete(FName SessionName, bool bWasSuccessful);
	USGMultiplayGameInstance();

	UFUNCTION(BlueprintCallable, Category = "MultiPlayer")
	void CreateServer();
	
	UFUNCTION(BlueprintCallable, Category = "MultiPlayer")
	void FindServers();
	
	UFUNCTION(BlueprintCallable, Category = "Multiplayer")
	void JoinServer(int32 SessionIndex);

protected:
	virtual void Init() override;
	
	// 방 생성 완료시 콜백 함수
	void OnCreateSessionComplete(FName Sessionname, bool bWasSuccessful);
	
	// 방 찾기 완료시 콜백 함수
	void OnFindSessionsComplete(bool bWasSuccessful);
	
	// 특정 방 접속 준비 완료시 콜백 함수
	void OnJoinSessionComplete(FName SessionName, EOnJoinSessionCompleteResult::Type Result);
	
private:
	// 엔진의 세션 기능을 담당할 세션 매니저 포인터
	IOnlineSessionPtr SessionInterface;
	
	TSharedPtr<FOnlineSessionSearch> SessionSearch;
	
	// 델리게이트 관련 변수
	FOnCreateSessionCompleteDelegate CreateSessionCompleteDelegate;
	FDelegateHandle CreateSessionCompleteDelegateHandle;
	
	FOnFindSessionsCompleteDelegate FindSessionsCompleteDelegate;
	FDelegateHandle FindSessionsCompleteDelegateHandle;
	
	FOnJoinSessionCompleteDelegate JoinSessionCompleteDelegate;
	FDelegateHandle JoinSessionCompleteDelegateHandle;
	
public:
	// 선택된 캐릭터 데이터를 저장할 변수
	UPROPERTY(BlueprintReadWrite)
	USGCharacterDataAsset* SelectedCharacterAsset;
};
