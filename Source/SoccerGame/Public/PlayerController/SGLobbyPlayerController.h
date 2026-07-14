// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "SoccerGame/Public/PlayerState/SGLobbyPlayerState.h" // ◀ 이 구조체가 정의된 헤더(예: PlayerState.h)를 반드시 여기에 추가!
#include "GameplayTagContainer.h" 
#include "SoccerGame/Public/UI/SGLobbyWidget.h"
#include "SGLobbyPlayerController.generated.h"

struct FSGPlayerLobbyInfo;
class UUserWidget;
/**
 * 
 */
UCLASS()
class SOCCERGAME_API ASGLobbyPlayerController : public APlayerController
{
	GENERATED_BODY()
public:
	
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
	
	UFUNCTION(BlueprintCallable, Category = "Lobby")
	void SellectReady();
	
	// 클라이언트가 UI 버튼 등을 눌렀을 때 호출하는 함수
	UFUNCTION(Server, Reliable, WithValidation)
	void RequestChangeTeam(FGameplayTag NewTeam);
	
	void Client_UpdateLobbyUI(const TArray<FSGPlayerLobbyInfo>& InPlayerInfos);
	
	void TimeUIUpdate(int32 NewTime);
	
	// 블루프린트에서 GameStart 버튼 누른 직후 호출할 UI 데이터 주입 함수
	UFUNCTION(BlueprintCallable, Category = "Lobby")
	void InitializeLocalPlayerLobbyUI();
	
	// 데이터 저장용
	void SaveDataToSubsystem();
protected:
	// [Server RPC] 준비 상태 변경 요청
	UFUNCTION(Server, Reliable, WithValidation)
	void Server_SetReady(bool bNewReadyState);
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "ASUIPlayerController")
	TSubclassOf<UUserWidget> UILobbyWidgetClass;

    UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "ASUIPlayerController")
    TObjectPtr<UUserWidget> UIWidgetInstance;
};
