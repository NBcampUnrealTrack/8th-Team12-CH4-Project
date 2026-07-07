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
	virtual void Tick(float DeltaTime) override;
    
	FORCEINLINE UStaticMeshComponent* GetSoccerBallMesh() const { return SoccerBallMesh; }

	// [추가] 슛을 차는 순간 클라에서 미리 서버 동기화 차단
	void IgnoreServerPhysicsForDuration(float Duration);
	
protected:
	// 플레이어나 무언가와 부딪혔을 때 호출되는 함수
	virtual void NotifyHit(
		class UPrimitiveComponent* MyComp, 
		AActor* Other, 
		class UPrimitiveComponent* OtherComp, 
		bool bSelfMoved, 
		FVector HitLocation, 
		FVector HitNormal, 
		FVector NormalImpulse, 
		const FHitResult& Hit
	) override;

private:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "SoccerBall", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UStaticMeshComponent> SoccerBallMesh;
    
	// 소유권 변경에 락을 거는 타이머
	UPROPERTY()
	FTimerHandle OwnerCooldownTimerHandle;
    
	// 공에서 발을 떼면 소유권을 다시 NULL로 회수할 자동 반납 타이머
	UPROPERTY()
	FTimerHandle OwnerReleaseTimerHandle;

	// 소유권 변경 락을 해제할 빈 함수
	void ResetOwnerCooldown();

	// 소유권을 NULL로 돌려놓을 함수
	void ReleaseOwner();
	
	// [추가] 슛 전용 로컬 예측 타이머
	float KickPredictionTimer = 0.0f;
};