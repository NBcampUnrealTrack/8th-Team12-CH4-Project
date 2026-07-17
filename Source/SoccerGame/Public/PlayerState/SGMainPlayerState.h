// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerState.h"
#include "GameplayTagContainer.h"
#include "SGMainPlayerState.generated.h"

USTRUCT(BlueprintType)
struct FPlayerBackupData
{
	GENERATED_BODY()
	
	// 이름
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Backup")
	FString PlayerName = TEXT("None");
    
	// 팀 선택
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Backup")
	FGameplayTag PlayerTeam = FGameplayTag::RequestGameplayTag(FName("Team.Waiting"));

	// 로비에서 선택한 캐릭터
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Backup")
	FGameplayTag SelectedCharacterTag;

	// 스코어 
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Backup")
	int32 Score = 0; 
};
UCLASS()
class SOCCERGAME_API ASGMainPlayerState : public APlayerState
{
	GENERATED_BODY()
public:
	ASGMainPlayerState();

	// 멀티플레이 동기화를 위한 변수 등록 함수
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	
	void CopyProperties(APlayerState* NewPlayerState);
	

protected:
	// 새 레벨 진입 시 호출 (서브시스템에서 데이터 복원)
	virtual void BeginPlay() override;

	// 레벨 이동으로 파괴 시 호출 (서브시스템에 데이터 백업)
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
	
	UFUNCTION()
	void OnRep_CustomPlayerName();

public:
	
	UPROPERTY(Replicated, BlueprintReadOnly, Category = "PlayerState")
	FString CustomPlayerName = "None";
	
	UPROPERTY(Replicated, BlueprintReadOnly, Category = "PlayerState")
	FGameplayTag CurrentTeamTag = FGameplayTag::RequestGameplayTag(FName("Team.Waiting"));

	UPROPERTY(Replicated, BlueprintReadOnly, Category = "PlayerState")
	int32 PlayerScore = 0;

	UPROPERTY(Replicated, BlueprintReadOnly, Category = "PlayerState|Character")
	FGameplayTag SelectedCharacterTag;
};
