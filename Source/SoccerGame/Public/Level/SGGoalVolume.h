#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "GameFramework/Actor.h"
#include "SGGoalVolume.generated.h"

class UBoxComponent;

UCLASS()
class SOCCERGAME_API ASGGoalVolume : public AActor
{
	GENERATED_BODY()
	
public:
	ASGGoalVolume();

protected:
	virtual void BeginPlay() override;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<UBoxComponent> GoalTrigger;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Goal", meta = (Categories = "Team"))
	FGameplayTag DefendingTeamTag;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Goal")
	bool bGoalEnabled = true;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Goal")
	float GoalCooldown = 1.0f;

	UFUNCTION()
	void OnGoalBeginOverlap(
		UPrimitiveComponent* OverlappedComponent,
		AActor* OtherActor,
		UPrimitiveComponent* OtherComp,
		int32 OtherBodyIndex,
		bool bFromSweep,
		const FHitResult& SweepResult
	);

	void HandleGoalScored();

private:
	FTimerHandle GoalCooldownTimerHandle;

	void EnableGoal();
};
