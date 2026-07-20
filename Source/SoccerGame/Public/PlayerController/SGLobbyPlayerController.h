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
class USGLobbyWidget;
class USGChangeUsernameWidget;
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
	virtual void OnRep_PlayerState() override;
	
	UFUNCTION(BlueprintCallable, Category = "Lobby")
	void SellectReady();
	
	// 클라이언트가 UI 버튼 등을 눌렀을 때 호출하는 함수
	UFUNCTION(Server, Reliable, WithValidation)
	void RequestChangeTeam(FGameplayTag NewTeam);

	// 캐릭터 선택 UI에서 호출하는 서버 변경 요청
	UFUNCTION(Server, Reliable, WithValidation, BlueprintCallable, Category = "Lobby|Character")
	void RequestChangeCharacter(FGameplayTag NewCharacterTag);
	
	UFUNCTION(Client, Reliable)
	void Client_UpdateLobbyUI(const TArray<FSGPlayerLobbyInfo>& InPlayerInfos);
	
	void TimeUIUpdate(int32 NewTime);
	
	// 블루프린트에서 GameStart 버튼 누른 직후 호출할 UI 데이터 주입 함수
	UFUNCTION(BlueprintCallable, Category = "Lobby")
	void InitializeLocalPlayerLobbyUI();
	
	// 데이터 저장용
	void SaveDataToSubsystem();
	
	void RequestChangeUsername(const FString& NewUsername);
	
	UFUNCTION(BlueprintCallable, Category = "Lobby|Username")
	void OpenChangeUsernameWidget();

	UFUNCTION(BlueprintCallable, Category = "Lobby|Username")
	void CloseChangeUsernameWidget();
	
	UFUNCTION(Client, Reliable)
	void ClientToMainMenu();

protected:
	// [Server RPC] 준비 상태 변경 요청
	UFUNCTION(Server, Reliable, WithValidation)
	void Server_SetReady(bool bNewReadyState);
	
	UFUNCTION(Server, Reliable)
	void Server_ChangeUsername(const FString& NewUsername);
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "ASUIPlayerController")
	TSubclassOf<UUserWidget> LobbyWidgetClass;

    UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "ASUIPlayerController")
    TObjectPtr<UUserWidget> LobbyWidgetInstance;
	
	UPROPERTY(EditDefaultsOnly,BlueprintReadOnly,Category = "UI|Username")
	TSubclassOf<UUserWidget>ChangeUsernameWidgetClass;

	UPROPERTY()
	TObjectPtr<UUserWidget>ChangeUsernameWidgetInstance;
};
