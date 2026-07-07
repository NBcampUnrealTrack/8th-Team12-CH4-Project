// SG_SoccerBall.h

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "SG_SoccerBall.generated.h"

UCLASS()
class SOCCERGAME_API ASG_SoccerBall : public AActor
{
	GENERATED_BODY()
	
public:
	ASG_SoccerBall();
	
	FORCEINLINE UStaticMeshComponent* GetSoccerBallMesh() const { return SoccerBallMesh; }

private:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "SoccerBall", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UStaticMeshComponent> SoccerBallMesh;

};
