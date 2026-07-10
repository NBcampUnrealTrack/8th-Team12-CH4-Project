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

UGA_SG_Kick::UGA_SG_Kick()
{
    InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;
    bReplicateInputDirectly = true;
   
   bRetriggerInstancedAbility = false;
   
}

void UGA_SG_Kick::ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData)
{
   UE_LOG(LogTemp, Warning, TEXT(">>> Kick ActivateAbility 호출됨!"));
   // 이미 발차기/차징중이면 취소
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
    
    if (GetWorld())
    {
       ChargeStartTime = GetWorld()->GetTimeSeconds();
    }
    
    UAbilityTask_WaitInputRelease* WaitInputReleaseTask = UAbilityTask_WaitInputRelease::WaitInputRelease(this, false);
    if (WaitInputReleaseTask)
    {
       WaitInputReleaseTask->ReadyForActivation();
    }
    else
    {
       EndAbility(Handle, ActorInfo, ActivationInfo, true, false);
    }
}

void UGA_SG_Kick::EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo,
    const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled)
{
   UE_LOG(LogTemp, Error, TEXT("<<< Kick EndAbility 호출됨!"));
   bIsKickInProgress = false;
   
   Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);
}

void UGA_SG_Kick::InputReleased(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo)
{
   if (!bIsKickInProgress)
   {
      return;
   }
   
    Super::InputReleased(Handle, ActorInfo, ActivationInfo);
    
    float CurrentTime = GetWorld() ? GetWorld()->GetTimeSeconds() : 0.0f;
    float ActualChargeTime = CurrentTime - ChargeStartTime;
    
    float BaseKickPower = 500.0f;
    UAbilitySystemComponent* ASC = GetAbilitySystemComponentFromActorInfo();
    if (ASC)
    {
       const UGAS_SG_CharacterAttributeSet* AttributeSet = ASC->GetSet<UGAS_SG_CharacterAttributeSet>();
       if (AttributeSet)
       {
          BaseKickPower = AttributeSet->GetKickPower();
       }
    }
    
    float ChargeRatio = FMath::Clamp(ActualChargeTime / MaxChargeTime, 0.0f, 1.0f);
    float FinalMultiplier = FMath::Lerp(1.0f, MaxPowerMultiplier, ChargeRatio);
    CachedFinalKickPower = BaseKickPower * FinalMultiplier; 
    
    UAnimMontage* MontageToPlay = (ActualChargeTime < ActionSplitTime) ? PassMontage : ShootMontage;

    if (MontageToPlay)
    {
       UAbilityTask_PlayMontageAndWait* PlayMontageTask = UAbilityTask_PlayMontageAndWait::CreatePlayMontageAndWaitProxy(
          this, NAME_None, MontageToPlay, 1.0f, NAME_None, true
       );
       
       if (PlayMontageTask)
       {
          PlayMontageTask->OnCompleted.AddDynamic(this, &UGA_SG_Kick::K2_EndAbility);
          PlayMontageTask->OnBlendOut.AddDynamic(this, &UGA_SG_Kick::K2_EndAbility);
          PlayMontageTask->OnInterrupted.AddDynamic(this, &UGA_SG_Kick::K2_EndAbility);
          PlayMontageTask->OnCancelled.AddDynamic(this, &UGA_SG_Kick::K2_EndAbility);
          PlayMontageTask->ReadyForActivation();
       }
       
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
       K2_EndAbility();
    }
}

void UGA_SG_Kick::FindAndPushBall()
{
    ACharacter* Character = Cast<ACharacter>(GetAvatarActorFromActorInfo());
    if (!Character)
    {
       return;
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
   UE_LOG(LogTemp, Warning, TEXT("[%s] 킥 오버랩 감지된 액터 수: %d"), *NetMode, OutActors.Num());

    if (bHit)
    {
       for (AActor* HitActor : OutActors)
       {
          if (!HitActor) continue;
          
          if (ASG_SoccerBall* SoccerBall = Cast<ASG_SoccerBall>(HitActor))
          {
             FVector DirToBall = (HitActor->GetActorLocation() - Character->GetActorLocation()).GetSafeNormal2D();
             float DotResult = FVector::DotProduct(Forward.GetSafeNormal2D(), DirToBall);

             // 캐릭터 전방(100도 범위 반원) 내에 있는 공만 판정
             if (DotResult < -0.2f) 
             {
                continue; 
             }
             
             UE_LOG(LogTemp, Warning, TEXT("[%s] 축구공 Push 실행"), *NetMode);
             
             UStaticMeshComponent* BallMesh = SoccerBall->GetBallMesh();
             if (BallMesh)
             {
                FVector PushDirection = (HitActor->GetActorLocation() - Character->GetActorLocation()).GetSafeNormal();
                PushDirection.Z += 0.7f; // 상향 각도 보정
                PushDirection = PushDirection.GetSafeNormal();
                
                FVector ImpulseVector = PushDirection * CachedFinalKickPower;
                
                // 🔴 서버인 경우: 소유권 넘기고 킥
                if (HasAuthority(&CurrentActivationInfo))
                {
                   SoccerBall->SetBallOwner(Character);
                   if (BallMesh->IsSimulatingPhysics())
                   {
                      BallMesh->AddImpulse(ImpulseVector, NAME_None, true);
                   }
                }
                // 🟢 로컬 클라이언트인 경우
                else
                {
                   // 내가 Owner라면 즉시 로컬 Impulse
                   if (SoccerBall->IsLocallyControlledOwner())
                   {
                      if (BallMesh->IsSimulatingPhysics())
                      {
                         BallMesh->AddImpulse(ImpulseVector, NAME_None, true);
                      }
                   }
                   // Owner가 서버(nullptr)라면 내 화면에서 예측을 위해 잠시 물리를 켜고 Impulse
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

bool UGA_SG_Kick::CanActivateAbility(
    const FGameplayAbilitySpecHandle Handle, 
    const FGameplayAbilityActorInfo* ActorInfo, 
    const FGameplayTagContainer* SourceTags, 
    const FGameplayTagContainer* TargetTags, 
    OUT FGameplayTagContainer* OptionalRelevantTags) const
{
   // 이미 킥이 진행 중이라면 서버든 클라든 활성화 자체를 거부함!
   if (bIsKickInProgress)
   {
      return false;
   }

   return Super::CanActivateAbility(Handle, ActorInfo, SourceTags, TargetTags, OptionalRelevantTags);
}
