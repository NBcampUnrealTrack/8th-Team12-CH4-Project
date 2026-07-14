// GA_SG_Kick.cpp

#include "Character/GA/GA_SG_Kick.h"
#include "GameFramework/Character.h"
#include "Kismet/KismetSystemLibrary.h"
#include "AbilitySystemComponent.h"
#include "Character/GAS/GAS_SG_CharacterAttributeSet.h"
#include "Abilities/Tasks/AbilityTask_WaitInputRelease.h"
#include "Abilities/Tasks/AbilityTask_PlayMontageAndWait.h"
#include "Abilities/Tasks/AbilityTask_WaitGameplayEvent.h"
#include "AbilitySystemGlobals.h"
#include "AbilitySystemInterface.h"
#include "Character/SG_SoccerBall.h"
#include "PlayerController/SGMainPlayerController.h"

UGA_SG_Kick::UGA_SG_Kick()
{
    InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;
    bReplicateInputDirectly = true;
    bRetriggerInstancedAbility = false;
}

bool UGA_SG_Kick::CanActivateAbility(
    const FGameplayAbilitySpecHandle Handle, 
    const FGameplayAbilityActorInfo* ActorInfo, 
    const FGameplayTagContainer* SourceTags, 
    const FGameplayTagContainer* TargetTags, 
    OUT FGameplayTagContainer* OptionalRelevantTags) const
{
   // 이미 킥이 진행 중이라면 서버든 클라든 활성화 자체를 거부
   if (bIsKickInProgress)
   {
      return false;
   }

   return Super::CanActivateAbility(Handle, ActorInfo, SourceTags, TargetTags, OptionalRelevantTags);
}

void UGA_SG_Kick::ActivateAbility(
    const FGameplayAbilitySpecHandle Handle, 
    const FGameplayAbilityActorInfo* ActorInfo, 
    const FGameplayAbilityActivationInfo ActivationInfo, 
    const FGameplayEventData* TriggerEventData)
{
    UE_LOG(LogTemp, Warning, TEXT(">>> Kick ActivateAbility 호출됨!"));

    if (bIsKickInProgress)
    {
        return;
    }
   
    Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);

    if (!CommitAbility(Handle, ActorInfo, ActivationInfo))
    {
        EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
        return;
    }
   
    bIsKickInProgress = true;

    // 발차기 몽타주 바로 재생
    if (KickMontage)
    {
        UAbilityTask_PlayMontageAndWait* PlayMontageTask = UAbilityTask_PlayMontageAndWait::CreatePlayMontageAndWaitProxy(
            this, NAME_None, KickMontage, 1.0f, NAME_None, true
        );
       
        if (PlayMontageTask)
        {
            PlayMontageTask->OnCompleted.AddDynamic(this, &UGA_SG_Kick::K2_EndAbility);
            PlayMontageTask->OnBlendOut.AddDynamic(this, &UGA_SG_Kick::K2_EndAbility);
            PlayMontageTask->OnInterrupted.AddDynamic(this, &UGA_SG_Kick::K2_EndAbility);
            PlayMontageTask->OnCancelled.AddDynamic(this, &UGA_SG_Kick::K2_EndAbility);
            PlayMontageTask->ReadyForActivation();
        }

        // ANS(타격 프레임)에서 전송할 태그 이벤트 대기
        FGameplayTagContainer AssetTagsContainer = GetAssetTags();
        FGameplayTag MyAbilityTag = AssetTagsContainer.Num() > 0 ? AssetTagsContainer.GetByIndex(0) : FGameplayTag::EmptyTag;

        UAbilityTask_WaitGameplayEvent* WaitEventTask = UAbilityTask_WaitGameplayEvent::WaitGameplayEvent(
            this, MyAbilityTag, nullptr, false
        );
       
        if (WaitEventTask)
        {
            WaitEventTask->EventReceived.AddDynamic(this, &UGA_SG_Kick::OnGameplayEventReceived);
            WaitEventTask->ReadyForActivation();
        }
    }
    else
    {
        EndAbility(Handle, ActorInfo, ActivationInfo, true, false);
    }
}

void UGA_SG_Kick::EndAbility(
    const FGameplayAbilitySpecHandle Handle, 
    const FGameplayAbilityActorInfo* ActorInfo,
    const FGameplayAbilityActivationInfo ActivationInfo, 
    bool bReplicateEndAbility, 
    bool bWasCancelled)
{
   UE_LOG(LogTemp, Error, TEXT("<<< Kick EndAbility 호출됨!"));
   bIsKickInProgress = false;

   Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);
}

void UGA_SG_Kick::FindAndPushBall()
{
    ACharacter* Character = Cast<ACharacter>(GetAvatarActorFromActorInfo());
    if (!Character)
    {
        return;
    }

    // 기본 킥 파워 계산 (AttributeSet 기반 고정값)
    float FinalKickPower = 0.f;
    UAbilitySystemComponent* ASC = GetAbilitySystemComponentFromActorInfo();
    if (ASC)
    {
        const UGAS_SG_CharacterAttributeSet* AttributeSet = ASC->GetSet<UGAS_SG_CharacterAttributeSet>();
        if (AttributeSet)
        {
            FinalKickPower = AttributeSet->GetKickPower()  * KickPowerMultiplier;
        }
    }
    
    FVector Forward = Character->GetActorForwardVector();
    FVector StartLoc = Character->GetActorLocation() + (Forward * 60.0f);

    TArray<TEnumAsByte<EObjectTypeQuery>> ObjectTypes;
    ObjectTypes.Add(UEngineTypes::ConvertToObjectType(ECollisionChannel::ECC_PhysicsBody));

    TArray<AActor*> ActorsToIgnore;
    ActorsToIgnore.Add(Character);

    TArray<AActor*> OutActors;

    // 반원/구체 오버랩 스캔
    bool bHit = UKismetSystemLibrary::SphereOverlapActors(
        GetWorld(), 
        StartLoc, 
        160.0f, 
        ObjectTypes, 
        nullptr, 
        ActorsToIgnore, 
        OutActors
    );
   
    FString NetMode = HasAuthority(&CurrentActivationInfo) ? TEXT("🔴 서버") : TEXT("🟢 클라이언트");

    if (bHit)
    {
        for (AActor* HitActor : OutActors)
        {
            if (!HitActor) continue;
          
            if (ASG_SoccerBall* SoccerBall = Cast<ASG_SoccerBall>(HitActor))
            {
                FVector DirToBall = (HitActor->GetActorLocation() - Character->GetActorLocation()).GetSafeNormal2D();
                float DotResult = FVector::DotProduct(Forward.GetSafeNormal2D(), DirToBall);

                // 전방 범위 내 공만 판정
                if (DotResult < -0.2f) 
                {
                    continue; 
                }
             
                UStaticMeshComponent* BallMesh = SoccerBall->GetBallMesh();
                if (BallMesh)
                {
                    FVector PushDirection = (HitActor->GetActorLocation() - Character->GetActorLocation()).GetSafeNormal();
                    PushDirection.Z += UpwardForceRatio; // 슛 궤적을 위한 상향 각도
                    PushDirection = PushDirection.GetSafeNormal();
                
                    FVector ImpulseVector = PushDirection * FinalKickPower;
                
                    // 🔴 서버: 소유권 넘기고 물리 적용
                    if (HasAuthority(&CurrentActivationInfo))
                    {
                        SoccerBall->SetBallOwner(Character);
                        if (BallMesh->IsSimulatingPhysics())
                        {
                            BallMesh->AddImpulse(ImpulseVector, NAME_None, true);
                        }
                    }
                    // 🟢 클라이언트: 로컬 예측 물리 적용
                    else
                    {
                        if (SoccerBall->IsLocallyControlledOwner())
                        {
                            if (BallMesh->IsSimulatingPhysics())
                            {
                                BallMesh->AddImpulse(ImpulseVector, NAME_None, true);
                            }
                        }
                        else
                        {
                            BallMesh->SetSimulatePhysics(true);
                            BallMesh->AddImpulse(ImpulseVector, NAME_None, true);
                        }
                    }
                }
            }
        }
    }
}

void UGA_SG_Kick::OnGameplayEventReceived(FGameplayEventData Payload)
{
    // 공 밀어내기 (서버 & 로컬 오너 연산)
    FindAndPushBall();
    
    // 사람 타격 및 데미지 처리는 서버에서만
    if (HasAuthority(&CurrentActivationInfo))
    {
       OnEnemyHitReceived(Payload);
    }
}

void UGA_SG_Kick::OnEnemyHitReceived(FGameplayEventData Payload)
{
    if (!HasAuthority(&CurrentActivationInfo))
    {
        return;
    }

    AActor* HitEnemy = const_cast<AActor*>(Payload.Target.Get());
    if (!HitEnemy || !DamageEffectClass)
    {
        return;
    }

    UAbilitySystemComponent* MyASC = GetAbilitySystemComponentFromActorInfo();
    UAbilitySystemComponent* TargetASC = nullptr;
    
    if (IAbilitySystemInterface* ASCInterface = Cast<IAbilitySystemInterface>(HitEnemy))
    {
        TargetASC = ASCInterface->GetAbilitySystemComponent();
    }
    else
    {
        TargetASC = UAbilitySystemGlobals::GetAbilitySystemComponentFromActor(HitEnemy);
    }

    if (!MyASC || !TargetASC)
    {
        return;
    }

    FGameplayEffectContextHandle EffectContext = MyASC->MakeEffectContext();
    EffectContext.AddSourceObject(this);

    FGameplayEffectSpecHandle NewHandle = MyASC->MakeOutgoingSpec(DamageEffectClass, 1.0f, EffectContext);
    
    if (NewHandle.IsValid())
    {
        MyASC->ApplyGameplayEffectSpecToTarget(*NewHandle.Data.Get(), TargetASC);
    }
}


