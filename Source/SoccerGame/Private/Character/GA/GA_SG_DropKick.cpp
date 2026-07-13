// GA_SG_DropKick.cpp

#include "Character/GA/GA_SG_DropKick.h"
#include "GameFramework/Character.h"
#include "Kismet/KismetSystemLibrary.h"
#include "AbilitySystemComponent.h"
#include "Character/GAS/GAS_SG_CharacterAttributeSet.h"
#include "Abilities/Tasks/AbilityTask_PlayMontageAndWait.h"
#include "Abilities/Tasks/AbilityTask_WaitGameplayEvent.h"
#include "AbilitySystemGlobals.h"
#include "AbilitySystemInterface.h"

UGA_SG_DropKick::UGA_SG_DropKick()
{
    InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;
    bReplicateInputDirectly = true;
}

void UGA_SG_DropKick::ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData)
{
    Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);

    if (!CommitAbility(Handle, ActorInfo, ActivationInfo))
    {
        EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
        return;
    }

    // 시작 시 중복 타격 배열 초기화
    AlreadyHitActors.Empty();

    if (DropKickMontage)
    {
        UAbilityTask_PlayMontageAndWait* PlayMontageTask = UAbilityTask_PlayMontageAndWait::CreatePlayMontageAndWaitProxy(
            this, NAME_None, DropKickMontage, 1.0f, NAME_None, true
        );
        if (PlayMontageTask)
        {
            PlayMontageTask->OnCompleted.AddDynamic(this, &UGA_SG_DropKick::K2_EndAbility);
            PlayMontageTask->OnBlendOut.AddDynamic(this, &UGA_SG_DropKick::K2_EndAbility);
            PlayMontageTask->OnInterrupted.AddDynamic(this, &UGA_SG_DropKick::K2_EndAbility);
            PlayMontageTask->OnCancelled.AddDynamic(this, &UGA_SG_DropKick::K2_EndAbility);
            PlayMontageTask->ReadyForActivation();
        }

        // 날아가는 동안 이벤트 대기
        FGameplayTagContainer AssetTagsContainer = GetAssetTags();
        FGameplayTag MyAbilityTag = AssetTagsContainer.Num() > 0 ? AssetTagsContainer.GetByIndex(0) : FGameplayTag::EmptyTag;
        
        UAbilityTask_WaitGameplayEvent* WaitEventTask = UAbilityTask_WaitGameplayEvent::WaitGameplayEvent(
            this,
            MyAbilityTag, 
            nullptr, 
            false
            );
        
        if (WaitEventTask)
        {
            WaitEventTask->EventReceived.AddDynamic(this, &UGA_SG_DropKick::OnGameplayEventReceived);
            WaitEventTask->ReadyForActivation();
        }
    }
    else
    {
        K2_EndAbility();
    }
}

void UGA_SG_DropKick::EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled)
{
    AlreadyHitActors.Empty();
    
    Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);
}

void UGA_SG_DropKick::OnGameplayEventReceived(FGameplayEventData Payload)
{
    // 드롭킥 도중 이벤트 발생
    AActor* HitTarget = const_cast<AActor*>(Payload.Target.Get());
    if (!HitTarget)
    {
        return;
    }
    UE_LOG(LogTemp, Log, TEXT("드롭킥 충돌 감지된 액터: %s"), *HitTarget->GetName());

    // 이미 맞은놈은 제외
    if (AlreadyHitActors.Contains(HitTarget))
    {
        return;
    }

    // 맞은놈 등록
    AlreadyHitActors.Add(HitTarget);

    // 공이면 PushBall 실행
    if (HitTarget->GetName().Contains(TEXT("SoccerBall")) || HitTarget->ActorHasTag(TEXT("Ball")))
    {
        PushBall(HitTarget);
    }
    else if (HasAuthority(&CurrentActivationInfo))
    {
        // 데미지 처리(서버에서)
        ApplyDamageToTarget(HitTarget, Payload);
    }
}

void UGA_SG_DropKick::PushBall(AActor* BallActor)
{
    ACharacter* Character = Cast<ACharacter>(GetAvatarActorFromActorInfo());
    if (!Character || !BallActor)
    {
        UE_LOG(LogTemp, Warning, TEXT("1"));
        return;
    }

    UStaticMeshComponent* BallMesh = Cast<UStaticMeshComponent>(BallActor->GetRootComponent());
    if (BallMesh && BallMesh->IsSimulatingPhysics())
    {
        float DropKickPower = 0.f;
        UAbilitySystemComponent* ASC = GetAbilitySystemComponentFromActorInfo();
        if (ASC)
        {
            const UGAS_SG_CharacterAttributeSet* AttributeSet = ASC->GetSet<UGAS_SG_CharacterAttributeSet>();
            if (AttributeSet)
            {
                DropKickPower = AttributeSet->GetKickPower() * KickPowerMultiplier;
            }
        }

        // 캐릭터가 날아가는 방향 기반으로 강하게 밀어내기
        FVector PushDirection = (BallActor->GetActorLocation() - Character->GetActorLocation()).GetSafeNormal();
        PushDirection.Z += UpwardForceRatio;
        PushDirection = PushDirection.GetSafeNormal();

        BallMesh->AddImpulse(PushDirection * DropKickPower, NAME_None, true);
        UE_LOG(LogTemp, Log, TEXT("쥰내 센 드롭킥 파워: %f"), DropKickPower);
    }
}

void UGA_SG_DropKick::ApplyDamageToTarget(AActor* HitEnemy, const FGameplayEventData& Payload)
{
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

    UE_LOG(LogTemp, Warning, TEXT("%s에게 드롭킥 날리기 성공"), *HitEnemy->GetName());

    FGameplayEffectContextHandle EffectContext = MyASC->MakeEffectContext();
    EffectContext.AddSourceObject(this);

    FGameplayEffectSpecHandle NewHandle = MyASC->MakeOutgoingSpec(DamageEffectClass, 1.0f, EffectContext);
    if (NewHandle.IsValid())
    {
        MyASC->ApplyGameplayEffectSpecToTarget(*NewHandle.Data.Get(), TargetASC);
    }
}