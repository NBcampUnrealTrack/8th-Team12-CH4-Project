#include "Level/SGGoalVolume.h"
#include "Components/BoxComponent.h"
#include "GameMode/SGMainGameMode.h"
#include "Kismet/GameplayStatics.h"

ASGGoalVolume::ASGGoalVolume()
{
    PrimaryActorTick.bCanEverTick = false;

    GoalTrigger = CreateDefaultSubobject<UBoxComponent>(TEXT("GoalTrigger"));
    RootComponent = GoalTrigger;

    GoalTrigger->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
    GoalTrigger->SetCollisionObjectType(ECC_WorldDynamic);
    GoalTrigger->SetCollisionResponseToAllChannels(ECR_Ignore);
    GoalTrigger->SetCollisionResponseToChannel(ECC_PhysicsBody, ECR_Overlap);
    GoalTrigger->SetGenerateOverlapEvents(true);
}

void ASGGoalVolume::BeginPlay()
{
    Super::BeginPlay();

    GoalTrigger->OnComponentBeginOverlap.AddDynamic(this, &ASGGoalVolume::OnGoalBeginOverlap);
}

// 오버랩
void ASGGoalVolume::OnGoalBeginOverlap(
    UPrimitiveComponent* OverlappedComponent,
    AActor* OtherActor,
    UPrimitiveComponent* OtherComp,
    int32 OtherBodyIndex,
    bool bFromSweep,
    const FHitResult& SweepResult
)
{
    if (!HasAuthority())
    {
        return;
    }

    if (!bGoalEnabled)
    {
        return;
    }

    if (!OtherActor)
    {
        return;
    }
    
    // 공인지 확인
    if (!OtherActor->ActorHasTag(TEXT("Ball")))
    {
        return;
    }

    HandleGoalScored();
}

void ASGGoalVolume::HandleGoalScored()
{
    if (!HasAuthority())
    {
        return;
    }

    bGoalEnabled = false;
    
    // 팀 확인
    bool bIsRedTeamGoal = DefendingTeamTag == FGameplayTag::RequestGameplayTag(FName("Team.Blue"));

    // GameMode에 알림
    ASGMainGameMode* GM = Cast<ASGMainGameMode>(UGameplayStatics::GetGameMode(this));
    if (GM)
    {
        GM->OnGoalScored(bIsRedTeamGoal);
        
        // 득점 팀 확인용 디버그
        GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Green, FString::Printf(TEXT("%d"), bIsRedTeamGoal));
    }

    // 골 먹힌 직후 트리거 비활성화 되도록 타이머 적용
    GetWorldTimerManager().SetTimer(
        GoalCooldownTimerHandle,
        this,
        &ASGGoalVolume::EnableGoal,
        GoalCooldown,
        false
    );
}

void ASGGoalVolume::EnableGoal()
{
    bGoalEnabled = true;
}