// SG_SoccerBall.cpp

#include "Character/SG_SoccerBall.h"
#include "GameFramework/Character.h"

ASG_SoccerBall::ASG_SoccerBall()
{
    
    PrimaryActorTick.bCanEverTick = true;
    
    bReplicates = true;
    SetReplicateMovement(true);

    SoccerBallMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("SoccerBallMesh"));
    RootComponent = SoccerBallMesh;

    // 물리 및 콜리전 기본 세팅
    SoccerBallMesh->SetSimulatePhysics(true);
    SoccerBallMesh->SetCollisionProfileName(TEXT("PhysicsBody"));
    // SoccerBallMesh->SetNotifyRigidBodyCollision(true);
    
    // 잔디밭 감속 마찰력 설정
    SoccerBallMesh->SetLinearDamping(0.5f);   
    SoccerBallMesh->SetAngularDamping(0.3f);  
    
    // 공 무게 
    SoccerBallMesh->SetMassOverrideInKg(NAME_None, 0.7f, true);

    NetUpdateFrequency = 120.0f;
    MinNetUpdateFrequency = 60.0f;
}

void ASG_SoccerBall::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
    
    // 서버권한으로 소유권 관리(공이 멈추면)
    if (HasAuthority())
    {
        // 주인이 있는 상태인데 공이 거의 멈췄다면 (속력이 5 이하)
        if (GetOwner() != nullptr && SoccerBallMesh->GetPhysicsLinearVelocity().Size() < 5.0f)
        {
            // 안전하게 소유권을 반납하고 타이머를 취소합니다.
            SetOwner(nullptr);
            GetWorld()->GetTimerManager().ClearTimer(OwnerReleaseTimerHandle);
            UE_LOG(LogTemp, Log, TEXT("🔴 [SERVER] 공이 정지하여 소유권을 반납"));
        }
    }
    
    // [추가] 슛/패스 직후
    if (!HasAuthority())
    {
        if (KickPredictionTimer > 0.0f)
        {
            KickPredictionTimer -= DeltaTime;
            
            // 0.4초동안은 서버와 동기화 하지않음
            SoccerBallMesh->bReplicatePhysicsToAutonomousProxy = false;
        }
        else
        {
            // 타이머가 끝나면 다시 서버의 물리 상태 동기화
            SoccerBallMesh->bReplicatePhysicsToAutonomousProxy = true;
        }
    }
    
    // 화면 좌측 상단에 현재 오너 상태 실시간 디버깅 출력
    if (GEngine)
    {
        AActor* CurrentOwner = GetOwner();
        FString OwnerName = CurrentOwner ? CurrentOwner->GetName() : TEXT("No Owner (NULL)");
        FString NetMode = HasAuthority() ? TEXT("서버") : TEXT("클라이언트");
        FColor DisplayColor = HasAuthority() ? FColor::Red : FColor::Green;

        GEngine->AddOnScreenDebugMessage(
            1, 0.0f, DisplayColor, 
            FString::Printf(TEXT("[%s] 공의 현재 Owner: %s"), *NetMode, *OwnerName)
        );
    }
}

void ASG_SoccerBall::NotifyHit(
    UPrimitiveComponent* MyComp, 
    AActor* Other, 
    UPrimitiveComponent* OtherComp, 
    bool bSelfMoved, 
    FVector HitLocation, 
    FVector HitNormal, 
    FVector NormalImpulse,
    const FHitResult& Hit
)
{
    Super::NotifyHit(MyComp, Other, OtherComp, bSelfMoved, HitLocation, HitNormal, NormalImpulse, Hit);

    ACharacter* HittingCharacter = Cast<ACharacter>(Other);
    
    // 소유권 변경 조작은 오직 서버 권한
    if (HittingCharacter && HasAuthority())
    {
        // 만약 소유권 전환 락(0.1초)이 걸려있다면
        if (GetWorld()->GetTimerManager().IsTimerActive(OwnerCooldownTimerHandle))
        {
            // 락이 걸려있더라도 현재 이 공의 주인인 플레이어가 계속 비비고 있는 거라면
            if (GetOwner() == HittingCharacter)
            {
                // 소유권 반납 타이머를 계속 3초 뒤로 (드리블 유지)
                float ReleaseDelay = 3.0f; 
                GetWorld()->GetTimerManager().SetTimer(OwnerReleaseTimerHandle, this, &ASG_SoccerBall::ReleaseOwner, ReleaseDelay, false);
            }
            return;
        }

        // 소유권 변경
        if (GetOwner() != HittingCharacter)
        {
            SetOwner(HittingCharacter);
            
            // 0.1초 동안은 짧게 소유권이 또 바뀌지 않도록 락
            float OwnerLockTime = 0.1f; 
            GetWorld()->GetTimerManager().SetTimer(OwnerCooldownTimerHandle, this, &ASG_SoccerBall::ResetOwnerCooldown, OwnerLockTime, false);
        }
        
        // 공을 건드린 시점부터 1초 동안 안 건드리면 소유권을 해제하는 타이머 예약/갱신
        float ReleaseDelay = 3.0f; 
        GetWorld()->GetTimerManager().SetTimer(OwnerReleaseTimerHandle, this, &ASG_SoccerBall::ReleaseOwner, ReleaseDelay, false);
    }
}

void ASG_SoccerBall::ReleaseOwner()
{
    if (HasAuthority())
    {
        // 공에서 발을 떼고 0.5초가 지나면 오너를 다시 날려버려 서버 공으로 만듭니다.
        SetOwner(nullptr);
        UE_LOG(LogTemp, Warning, TEXT("🔴 [SERVER] 소유권 회수 (NULL)"));
    }
}

void ASG_SoccerBall::IgnoreServerPhysicsForDuration(float Duration)
{
    if (!HasAuthority())
    {
        KickPredictionTimer = Duration;
    }
}

void ASG_SoccerBall::ResetOwnerCooldown()
{
    // 타이머 구동용 빈 함수
}

