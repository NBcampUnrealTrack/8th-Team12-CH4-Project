#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerStart.h"
#include "SGPlayerStart.generated.h"

// 팀 식별용 enum class
UENUM(BlueprintType)
enum class ETeamId : uint8
{
	None UMETA(DisplayName = "None"),
	Blue UMETA(DisplayName = "Blue"),
	Red UMETA(DisplayName = "Red")
};

UCLASS()
class SOCCERGAME_API ASGPlayerStart : public APlayerStart
{
	GENERATED_BODY()
	
public:
	// 팀 Id: Blue / Red
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Spawn")
	ETeamId TeamId = ETeamId::None;
	
	// 스폰 인덱스값
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Spawn")
	int32 SpawnIndex = 0;
	
	// 초기 스폰에 사용할 건지
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Spawn")
	bool bUseForInitialSpawn = true;
	
	// 리스폰 시 사용할 건지
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Spawn")
	bool bUseForRespawn = true;
};
