// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerState.h"
#include "GameplayTagContainer.h"
#include "SGLobbyPlayerState.generated.h"

/**
 * 
 */
UCLASS()
class SOCCERGAME_API ASGLobbyPlayerState : public APlayerState
{
	GENERATED_BODY()
	
public:
	ASGLobbyPlayerState();
	
	// 멀티플레이 동기화를 위한 변수 등록 함수
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

protected:
	// 레벨 이동으로 파괴 시 호출 (데이터 백업)
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

public:
	UPROPERTY(Replicated, BlueprintReadOnly, Category = "Lobby Settings")
	FString CustomPlayerName = TEXT("UnknownPlayer");

	UPROPERTY(ReplicatedUsing = OnRep_ChangeTeam, BlueprintReadOnly,Category = "Lobby Settings")
	FGameplayTag CurrentTeamTag =FGameplayTag::RequestGameplayTag(FName("Team.Waiting"));

	UPROPERTY(Replicated, BlueprintReadOnly, Category = "Lobby Settings")
	int32 LobbyScore = 0;
	
	// 서버가 이 변수를 바꾸면 온렙 함수가 클라이언트에서 자동 호출됩니다.
	UPROPERTY(ReplicatedUsing = OnRep_IsReady, BlueprintReadOnly, Category = "SG_Lobby")
	bool bIsReady = false;
	
	// Getter 함수들
	FGameplayTag GetTeamTag() const { return CurrentTeamTag; }
	bool IsReady() const { return bIsReady; }

	// 서버에서 데이터를 직접 바꿀 때 사용할 함수들
	void SetReadyState(bool bNewReadyState);
	void SetTeamInternal(const FGameplayTag& SelectTeamTag);
protected:

	// 이름이 서버로부터 동기화되었을 때 클라이언트에서 실행될 함수
	UFUNCTION()
	void OnRep_CustomPlayerName();
    
	// 레디 상태가 동기화되었을 때 클라이언트에서 실행될 함수
	UFUNCTION()
	void OnRep_IsReady();
	
	UFUNCTION()
	void OnRep_ChangeTeam();
	
	void CopyProperties(APlayerState* NewPlayerState);
};
