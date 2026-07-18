#include "Level/SGGoalVolume.h"
#include "NiagaraFunctionLibrary.h"
#include "Components/BoxComponent.h"
#include "GameMode/SGMainGameMode.h"
#include "Kismet/GameplayStatics.h"

ASGGoalVolume::ASGGoalVolume()
{
    bReplicates = true;
    SetReplicateMovement(false);
    
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
    // 서버없는데 서버 호출하고 있음 . 
    UE_LOG(LogTemp, Warning, TEXT("GOAL BEGIN OVERLAP"));
    if (!HasAuthority())
    {
        
        return;
    }
    GEngine->AddOnScreenDebugMessage(
                -1,5.0f,FColor::Green,TEXT("Return"));
    if (!bGoalEnabled)
    {
        GEngine->AddOnScreenDebugMessage(
            -1,5.0f,FColor::Green,TEXT("Return2"));
        return;
    }

    if (!OtherActor)
    {
        GEngine->AddOnScreenDebugMessage(
            -1,5.0f,FColor::Green,TEXT("Return3"));
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
    GEngine->AddOnScreenDebugMessage(
               -1,5.0f,FColor::Green,TEXT("Ball"));
    bGoalEnabled = false;
    
    // 팀 확인
    //bool bIsRedTeamGoal = DefendingTeamTag == FGameplayTag::RequestGameplayTag(FName("Team.Blue"));

    // GameMode에 알림
    ASGMainGameMode* GM = Cast<ASGMainGameMode>(UGameplayStatics::GetGameMode(this));
    GEngine->AddOnScreenDebugMessage(
           -1,5.0f,FColor::Green,TEXT("Ball"));
    if (GM)
    {
        UE_LOG(LogTemp, Log, TEXT("HandleGoalScore  : %s ]") 
        , *DefendingTeamTag.ToString());
        //UE_LOG(LogTemp, Warning, TEXT("Goal Scores%s"),DefendingTeamTag);
        GM->OnGoalScored(DefendingTeamTag);
        
        // 득점 팀 확인용 디버그
        //GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Green, FString::Printf(TEXT("%d")));
    }
    
    // 골 VFX
    MulticastPlayGoalVFX();

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

void ASGGoalVolume::MulticastPlayGoalVFX_Implementation()
{
    if (!GoalVFX)
    {
        return;
    }
    
    UNiagaraFunctionLibrary::SpawnSystemAtLocation(
        this,
        GoalVFX,
        GetActorLocation(),
        GetActorRotation(),
        FVector(1),
        true,
        true,
        ENCPoolMethod::AutoRelease,
        true
    );
}
