// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "SGLobbyPlayerController.generated.h"

/**
 * 
 */
UCLASS()
class SOCCERGAME_API ASGLobbyPlayerController : public APlayerController
{
	GENERATED_BODY()
public:
	
	virtual void BeginPlay() override;
	// 로비 UI 버튼 등에서 호출할 함수 (레디 상태 토글)
	UFUNCTION(BlueprintCallable, Category = "Lobby")
	void ToggleReady();

protected:
	// [Server RPC] 준비 상태 변경 요청
	UFUNCTION(Server, Reliable, WithValidation)
	void Server_SetReady(bool bNewReadyState);
	
private:
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = ASUIPlayerController, Meta = (AllowPrivateAccess))
	TSubclassOf<UUserWidget> UIWidgetClass;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = ASUIPlayerController, Meta = (AllowPrivateAccess))
	TObjectPtr<UUserWidget> UIWidgetInstance;
};
