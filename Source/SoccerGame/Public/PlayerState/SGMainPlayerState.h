// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerState.h"
#include "SGMainPlayerState.generated.h"

UENUM(BlueprintType)
enum class ESGPlayerTeam : uint8
{
	Neutrality, // 중립 ( 팀 선택 안함 ) 
	BlueTeam, //  레드팀
	RedTeam // 블루 팀
};

USTRUCT(BlueprintType)
struct FPlayerBackupData
{
	GENERATED_BODY()

	// 플레이어 이름
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString PlayerName = "None";
	
	// 플레이어 팀 
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	ESGPlayerTeam PlayerTeam = ESGPlayerTeam::Neutrality;

	// 획득 스코어 
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 Socre = 0;
};
UCLASS()
class SOCCERGAME_API ASGMainPlayerState : public APlayerState
{
	GENERATED_BODY()
public:
	ASGMainPlayerState();

	// 멀티플레이 동기화를 위한 변수 등록 함수
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

protected:
	// 새 레벨 진입 시 호출 (서브시스템에서 데이터 복원)
	virtual void BeginPlay() override;

	// 레벨 이동으로 파괴 시 호출 (서브시스템에 데이터 백업)
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

public:
	// ----------------------------------------------------
	// 개별 관리 및 리플리케이션되는 멤버 변수들
	// ----------------------------------------------------
	UPROPERTY(Replicated, BlueprintReadOnly, Category = "PlayerState")
	ESGPlayerTeam CurrentTeam = ESGPlayerTeam::Neutrality;

	UPROPERTY(Replicated, BlueprintReadOnly, Category = "PlayerState")
	int32 PlayerScore = 0;
};
