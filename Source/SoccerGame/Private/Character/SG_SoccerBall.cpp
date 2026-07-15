// SG_SoccerBall.cpp

#include "Character/SG_SoccerBall.h"
#include "GameFramework/Character.h"
#include "HAL/PlatformStackWalk.h"
#include "Components/StaticMeshComponent.h"
#include "Net/UnrealNetwork.h"

ASG_SoccerBall::ASG_SoccerBall()
{
    
    PrimaryActorTick.bCanEverTick = true;
    
    // 메시 설정
    BallMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("SoccerBallMesh"));
    RootComponent = BallMesh;

    // ------------------------------- 공의 물리, 콜리전 등 기본 설정 ------------------------------- //
    // 물리 및 콜리전 기본 세팅
    BallMesh->SetSimulatePhysics(true);
    BallMesh->SetCollisionProfileName(TEXT("PhysicsBody"));
    BallMesh->SetNotifyRigidBodyCollision(true);
    // 잔디밭 감속 마찰력 설정
    BallMesh->SetLinearDamping(0.5f);   
    BallMesh->SetAngularDamping(0.3f);  
    // 공이 속도를 받아 바닥이나 벽을 뚫고 맵 밑으로 추락하는 것을 방지(어제 겪은 버그 방지...)
    BallMesh->SetUseCCD(true); 
    
    // ------------------------------------- 공의 네트워크 설정 ------------------------------------- //
    // 네트워크 복제 설정
    bReplicates = true;
    SetReplicateMovement(false);
    // 네트워크 업데이트 빈도 설정, 근데 SetReplicateMovement(false)면 사실 의미없을듯
    SetNetUpdateFrequency(120.f);
    SetMinNetUpdateFrequency(60.f);
    
    // -------------------------------------- 기본 변수 초기화 -------------------------------------- //
    StateSendInterval = 1.f / 30.f;
    StateSendTimer = 0.f;
    OwnershipDuration = 0.35f;
}

void ASG_SoccerBall::PostInitializeComponents()
{
    Super::PostInitializeComponents();

    if (BallMesh)
    {
        // 공 무게 
        BallMesh->SetMassOverrideInKg(NAME_None, 50.f, true);
    }
}

void ASG_SoccerBall::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
    
    if (IsLocallyControlledOwner())
    {
        UpdateOwnedBall(DeltaTime);
    }
    else if (HasAuthority())
    {
        UpdateServerBall(DeltaTime);
    }
    else
    {
        UpdateRemoteBall(DeltaTime);
    }
    
    if (HasAuthority())
    {
        if (HasBallOwner())
        {
            if (UWorld* World = GetWorld())
            {
                // Owner를 가지는 최소한의 시간 보장 (0.35초)
                if (World->GetTimeSeconds() > LastOwnerChangeTime + OwnershipDuration)
                {
                    float MinKeepOwnerTime = 0.5f; 
    
                    // Kick을 한 후 Onwer를 가지는 최소한의 시간 보장 (0.5초)
                    if (World->GetTimeSeconds() > LastOwnerChangeTime + MinKeepOwnerTime)
                    {
                        // 공의 속도가 거의 멈췄을 때만 Owner를 nullptr로 회수
                        if (BallMesh->GetPhysicsLinearVelocity().SizeSquared() < (50.f * 50.f)) // 50cm/s 이하
                        {
                            SetBallOwner(nullptr);
                        }
                    }
                }
            }
        }
    }
    
    // 현재 오너 상태 디버깅 출력
    if (GEngine)
    {
        AActor* CurrentOwner = GetOwner();
        FString OwnerName = CurrentOwner ? CurrentOwner->GetName() : TEXT("No Owner (NULL)");
        FString NetMode = HasAuthority() ? TEXT("🔴 서버") : TEXT("🟢 클라이언트");
        FColor DisplayColor = HasAuthority() ? FColor::Red : FColor::Green;

        GEngine->AddOnScreenDebugMessage(
            1, 0.0f, DisplayColor, 
            FString::Printf(TEXT("[%s] 공의 현재 Owner: %s"), *NetMode, *OwnerName)
        );
    }
}

void ASG_SoccerBall::BeginPlay()
{
    Super::BeginPlay();
    
    RefreshPhysicsSimulation();
    
    // 서버에서만 충돌 이벤트를 감지하여 Owner를 변경
    if (HasAuthority() && BallMesh)
    {
        BallMesh->OnComponentHit.AddDynamic(this, &ASG_SoccerBall::OnBallHit);
    }
}

void ASG_SoccerBall::OnRep_Owner()
{
    Super::OnRep_Owner();
    
    // 클라이언트에서 Owner가 바뀌었을 때 실행되는지 로그 확인
    FString NetMode = HasAuthority() ? TEXT("🔴 서버") : TEXT("🟢 클라이언트");
    UE_LOG(LogTemp, Warning, TEXT("[%s] OnRep_Owner 호출됨! 새로운 Owner: %s"), 
        *NetMode, GetOwner() ? *GetOwner()->GetName() : TEXT("NULL"));
    
    RefreshPhysicsSimulation();
}

void ASG_SoccerBall::OnRep_BallState()
{
    // 로컬 Owner는 제외
    if(IsLocallyControlledOwner())
    {
        return;
    }
    
    // Target만 저장, SetActorLocation X
    TargetLocation = ReplicatedBallState.Location + ReplicatedBallState.LinearVelocity * PredictionTime;
    TargetRotation = ReplicatedBallState.Rotation;
    TargetLinearVelocity = ReplicatedBallState.LinearVelocity;
    TargetAngularVelocity = ReplicatedBallState.AngularVelocity;
}

void ASG_SoccerBall::UpdateOwnedBall(float DeltaTime)
{
    StateSendTimer += DeltaTime;
    if (StateSendTimer < StateSendInterval)
    {
        return;
    }
    
    StateSendTimer = 0.f;
    FBallState State;
    FillCurrentBallState(State);
    Server_SendBallState(State);
}

void ASG_SoccerBall::UpdateServerBall(float DeltaTime)
{
    if (HasBallOwner())
    {
        return;
    }
    // Owner가 없으면 Server에서 Physics를 읽기만 한다.
    FillCurrentBallState(ReplicatedBallState);
}

void ASG_SoccerBall::UpdateRemoteBall(float DeltaTime)
{
    // 예측 위치(타겟 위치와 속도 등으로 계산)
    // FVector PredictedLocation =TargetLocation + (TargetLinearVelocity * PredictionTime);
    const FVector CurrentLocation = BallMesh->GetComponentLocation();
    const float Error = FVector::Distance(CurrentLocation, TargetLocation);
    
    // 공의 오차가 너무 크면
    if (Error > SnapDistance)
    {
        BallMesh->SetWorldLocationAndRotation(TargetLocation, TargetRotation);
        return;
    }
    
    FVector NewLocation = FMath::VInterpTo(
            BallMesh->GetComponentLocation(),
            TargetLocation,
            DeltaTime,
            PositionInterpSpeed
            );

    FRotator NewRotation = FMath::RInterpTo(
            BallMesh->GetComponentRotation(),
            TargetRotation,
            DeltaTime,
            RotationInterpSpeed
            );

    BallMesh->SetWorldLocationAndRotation(NewLocation, NewRotation);
    // BallMesh->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
    // if(bShouldSimulate)
    // {
    //     BallMesh->SetCollisionEnabled(
    //         ECollisionEnabled::QueryAndPhysics);
    // }
    // else
    // {
    //     BallMesh->SetCollisionEnabled(
    //         ECollisionEnabled::QueryOnly);
    // }
}

void ASG_SoccerBall::FillCurrentBallState(FBallState& OutState)
{
    OutState.Location = BallMesh->GetComponentLocation();
    OutState.Rotation = BallMesh->GetComponentRotation();
    OutState.LinearVelocity = BallMesh->GetPhysicsLinearVelocity();
    OutState.AngularVelocity = BallMesh->GetPhysicsAngularVelocityInDegrees();
}

void ASG_SoccerBall::RefreshPhysicsSimulation()
{
    // Server 혹은 Owner Client 일 경우에만 True!!!
    bool bShouldSimulate = HasAuthority() || IsLocallyControlledOwner();
    
    // Remote Client는 Physics를 OFF 해준다.
    BallMesh->SetSimulatePhysics(bShouldSimulate);
    if (!bShouldSimulate)
    {
        BallMesh->SetPhysicsLinearVelocity(FVector::ZeroVector);
        BallMesh->SetPhysicsAngularVelocityInDegrees(FVector::ZeroVector);
    }
    
    GEngine->AddOnScreenDebugMessage(
                -1, 3.0f, FColor::Red,
                FString::Printf(TEXT("PhysicsEnabled : %hhd"), bShouldSimulate)
            );
}


bool ASG_SoccerBall::IsLocallyControlledOwner() const
{
    if (!GetOwner())
    {
        return false;
    }
    
    const APawn* Pawn = Cast<APawn>(GetOwner());
    if (!Pawn)
    {
        return false;
    }
    
    return Pawn->IsLocallyControlled();
}

void ASG_SoccerBall::GetLifetimeReplicatedProps(
    TArray<FLifetimeProperty>& OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);

    DOREPLIFETIME(ASG_SoccerBall, ReplicatedBallState);
}

void ASG_SoccerBall::Server_SendBallState_Implementation(const FBallState& State)
{
    // 구조체 변수 업데이트
    ReplicatedBallState.Location = State.Location;
    ReplicatedBallState.LinearVelocity = State.LinearVelocity;
    ReplicatedBallState.AngularVelocity = State.AngularVelocity;
    ReplicatedBallState.Rotation = State.Rotation;
    
    // 서버의 공 위치 및 속도 업데이트
    BallMesh->SetWorldLocationAndRotation(State.Location, State.Rotation);
    BallMesh->SetPhysicsLinearVelocity(State.LinearVelocity);
    BallMesh->SetPhysicsAngularVelocityInDegrees(State.AngularVelocity);
    
    // 공이 Sleep 상태일 수 있어서 깨워준다
    BallMesh->WakeRigidBody();
}

bool ASG_SoccerBall::HasBallOwner() const
{
    return GetOwner()!=nullptr;
}

void ASG_SoccerBall::SetBallOwner(APawn* NewOwner)
{
    // Owner는 서버에서만 실행
    if (!HasAuthority())
    {
        return;
    }
    
    SetOwner(NewOwner);
    RefreshPhysicsSimulation();
    ForceNetUpdate();
    
    LastOwnerChangeTime =GetWorld()->TimeSeconds;
}

void ASG_SoccerBall::OnBallHit(
    UPrimitiveComponent* HitComponent, 
    AActor* OtherActor, 
    UPrimitiveComponent* OtherComp, 
    FVector NormalImpulse, 
    const FHitResult& Hit)
{
    // 서버가 아니거나, 충돌한 대상이 없으면 리턴
    if (!HasAuthority() || !OtherActor)
    {
        return;
    }

    // 충돌한 Actor가 캐릭터(APawn 또는 ACharacter)인지 확인
    APawn* TouchingPawn = Cast<APawn>(OtherActor);
    if (!TouchingPawn)
    {
        return;
    }

    // 이미 현재 Owner인 캐릭터가 또 친 거라면 무시
    if (TouchingPawn == GetOwner())
    {
        return;
    }
    
    // 충돌 시 가해진 충격량(Impact)의 크기 계산
    float ImpactForce = NormalImpulse.Size();
    float StrongImpactThreshold = 1000.f; 

    // 쿨이 지났거나 그냥 공에 비비는 것이 아닌 Kick 이상의 타격이 오면 Owner 변경
    if (CanChangeOwner() || ImpactForce > StrongImpactThreshold)
    {
        // 즉시 Owner 부여
        SetBallOwner(TouchingPawn);
        
        if (GEngine)
        {
            GEngine->AddOnScreenDebugMessage(
                2, 2.0f, FColor::Yellow,
                FString::Printf(TEXT("새로운 Owner : %s, ImpactForce : %f"), *TouchingPawn->GetName(), ImpactForce)
            );
        }
    }
}

bool ASG_SoccerBall::CanChangeOwner() const
{
    if (!GetWorld()) return false;

    // Onwer가 바뀔 수 있는 상태인지 체크
    return (GetWorld()->GetTimeSeconds() - LastOwnerChangeTime) >= OwnershipDuration;
}
