// SG_SoccerBall.cpp

#include "Character/SG_SoccerBall.h"

ASG_SoccerBall::ASG_SoccerBall()
{
 	
	PrimaryActorTick.bCanEverTick = true;
	
	// 네트워크 복제 활성화
	bReplicates = true;
	SetReplicateMovement(true);

	SoccerBallMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("SoccerBallMesh"));
	RootComponent = SoccerBallMesh;

	// 물리 및 콜리전 설정 추가
	SoccerBallMesh->SetSimulatePhysics(true);
	SoccerBallMesh->SetCollisionProfileName(TEXT("PhysicsBody"));
}


