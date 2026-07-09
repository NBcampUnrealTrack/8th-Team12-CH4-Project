// SG_SoccerBall.h

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "SG_SoccerBall.generated.h"

// -------------------------------------- 공 물리 설계 -------------------------------------- //
// 기본적으로 서버에서 공을 관리, 공을 터치하면 서버가 Owner를 부여하고 해당 클라의 공의 상태를 서버로 복제
// 기본 Physics는 Tick으로 매번 복제하지만 위치, 속도는 복제 횟수를 줄여서 서버의 부하를 줄인다.
// 
// Owner를 제외한 다른 클라에서는 위 복제값을 이용하여 (Prediction + Interpolation)을 실행해서 자연스러운 움직임을 보여주게 설정
// 며칠동안 공만 보니까 토나온다.


// 복제될 축구공 상태를 담은 구조체
USTRUCT(BlueprintType)
struct FBallState
{
	GENERATED_BODY()

	UPROPERTY()
	FVector Location;
	UPROPERTY()
	FVector LinearVelocity;
	UPROPERTY()
	FVector AngularVelocity;
	UPROPERTY()
	FRotator Rotation;
};

UCLASS()
class SOCCERGAME_API ASG_SoccerBall : public AActor
{
	GENERATED_BODY()
    
public:
	ASG_SoccerBall();
	virtual void Tick(float DeltaTime) override;
	virtual void BeginPlay() override;
	
	FORCEINLINE UStaticMeshComponent* GetBallMesh() const { return BallMesh; }

protected:
	// ----------------------------------- Replicate Ball ----------------------------------- //
	// 축구공 상태 변수
	FVector TargetLocation;
	FRotator TargetRotation;
	FVector TargetLinearVelocity;
	FVector TargetAngularVelocity;
	
	UPROPERTY(ReplicatedUsing=OnRep_BallState)
	FBallState ReplicatedBallState;
	
	UFUNCTION()
	void OnRep_BallState();
	
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	
	// ----------------------------------------- RPC ----------------------------------------- //
	// Unreliable사용, 속도 우선
	UFUNCTION(Server, Unreliable)
	void Server_SendBallState(const FBallState& State);
	
	// --------------------------------------- Owner, Ball --------------------------------------- //
public:
	// Owner 판정 함수
	bool IsLocallyControlledOwner() const;
	// Ball Owner 설정
	void SetBallOwner(APawn* NewOwner);
	
protected:
	// Owner 유무 리턴 함수
	bool HasBallOwner() const;
	// Local Ball의 Physics 결과를 서버에 전송
	void UpdateOwnedBall(float DeltaTime);
	// Server Ball의 Physics를 읽는다
	void UpdateServerBall(float DeltaTime);
	// Remote Ball의 위치, 회전을 보간한다
	void UpdateRemoteBall(float DeltaTime);
	// 현재 공의 Physics 상태 읽기
	void FillCurrentBallState(FBallState& OutState);
	
	// -------------------------------------- Owner, Physics -------------------------------------- //
	// Physics 설정을 바꾸는 함수
	void RefreshPhysicsSimulation();
	// Owner가 Set되고 Replicate 될 때 호출되는 함수
	virtual void OnRep_Owner() override;
	
	// ------------------------------------- Ball, Collision ------------------------------------- //
	// 공이 무언가와 충돌했을 때 호출될 함수 (Physics Body Hit)
	UFUNCTION()
	void OnBallHit(
		UPrimitiveComponent* HitComponent, 
		AActor* OtherActor, 
		UPrimitiveComponent* OtherComp, 
		FVector NormalImpulse, 
		const FHitResult& Hit
	);

	// 새로운 Owner를 설정할 수 있는 상태인지 (쿨다운 체크)
	bool CanChangeOwner() const;

private:
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "SoccerBall", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UStaticMeshComponent> BallMesh;
	
	// --------------------------------------- Variable --------------------------------------- //
	// 마지막 Owner가 바뀐 시간
	float LastOwnerChangeTime;
	// 상태 전송 보간
	float StateSendInterval;
	// 상태 전송 타이머
	float StateSendTimer;
	// Owner 유지 시간
	float OwnershipDuration;
	
	UPROPERTY(EditAnywhere, Category="Network")
	float PositionInterpSpeed = 10.f;
	UPROPERTY(EditAnywhere, Category="Network")
	float RotationInterpSpeed = 10.f;
	UPROPERTY(EditAnywhere, Category="Network")
	float PredictionTime = 0.03f;
	UPROPERTY(EditAnywhere, Category="Network")
	float SnapDistance = 300.f;
};
