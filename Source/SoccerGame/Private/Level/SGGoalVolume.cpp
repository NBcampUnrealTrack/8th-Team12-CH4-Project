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
    
    bool bIsRedTeamGoal = DefendingTeamTag == FGameplayTag::RequestGameplayTag(FName("Team.Blue"));

    // GameMode에 알림
    ASGMainGameMode* GM = Cast<ASGMainGameMode>(UGameplayStatics::GetGameMode(this));
    if (GM)
    {
        GM->OnGoalScored(bIsRedTeamGoal);
    }

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