// GA_SG_DropKick.cpp

#include "Character/GA/GA_SG_DropKick.h"
#include "GameFramework/Character.h"
#include "AbilitySystemComponent.h"
#include "Character/GAS/GAS_SG_CharacterAttributeSet.h"
#include "Abilities/Tasks/AbilityTask_PlayMontageAndWait.h"
#include "Abilities/Tasks/AbilityTask_WaitGameplayEvent.h"
#include "AbilitySystemGlobals.h"
#include "AbilitySystemInterface.h"
#include "Kismet/GameplayStatics.h"
#include "Character/SG_Character.h"
#include "PlayerState/SGMainPlayerState.h"

UGA_SG_DropKick::UGA_SG_DropKick()
{
    InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;
    bReplicateInputDirectly = true;
}

bool UGA_SG_DropKick::CanActivateAbility(
    const FGameplayAbilitySpecHandle Handle,
    const FGameplayAbilityActorInfo* ActorInfo, 
    const FGameplayTagContainer* SourceTags,
    const FGameplayTagContainer* TargetTags,
    FGameplayTagContainer* OptionalRelevantTags) const
{
    if (!ActorInfo || !ActorInfo->AbilitySystemComponent.IsValid())
    {
        return false;
    }

    UAbilitySystemComponent* ASC = ActorInfo->AbilitySystemComponent.Get();
    FGameplayTag ImmunityTag = FGameplayTag::RequestGameplayTag(FName("State.Immunity"));
    bool bHasImmunity = ASC->HasMatchingGameplayTag(ImmunityTag);

    if (bHasImmunity)
    {
        return false;
    }
    
    return Super::CanActivateAbility(Handle, ActorInfo, SourceTags, TargetTags, OptionalRelevantTags);
}

void UGA_SG_DropKick::ActivateAbility(
    const FGameplayAbilitySpecHandle Handle, 
    const FGameplayAbilityActorInfo* ActorInfo, 
    const FGameplayAbilityActivationInfo ActivationInfo, 
    const FGameplayEventData* TriggerEventData)
{
    Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);

    if (!CommitAbility(Handle, ActorInfo, ActivationInfo))
    {
        EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
        return;
    }
    
    ASG_Character* Character = Cast<ASG_Character>(GetAvatarActorFromActorInfo());
    if (!Character)
    {
        EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
        return;
    }
     
    // 현재 캐릭터의 수평 방향(Yaw)과 카메라/컨트롤러의 수평 방향(Yaw) 추출
    const float CurrentActorYaw = Character->GetActorRotation().Yaw;
    const float CameraYaw = Character->GetControlRotation().Yaw;

    // 두 방향 사이의 최단 각도 차이 계산 (-180 ~ 180 도 범위로 반환)
    const float DeltaYaw = FRotator::NormalizeAxis(CameraYaw - CurrentActorYaw);

    // 각도 차이를 최대 -90도 ~ +90도 사이로 제한 (Clamp)
    const float ClampedDeltaYaw = FMath::Clamp(DeltaYaw, -90.0f, 90.0f);

    // 캐릭터 정면 기준 제한된 각도만큼 회전된 최종 Yaw 적용
    const float FinalTargetYaw = FRotator::NormalizeAxis(CurrentActorYaw + ClampedDeltaYaw);
    const FRotator TargetRotation(0.0f, FinalTargetYaw, 0.0f);

    Character->SetActorRotation(TargetRotation);

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

void UGA_SG_DropKick::EndAbility(
    const FGameplayAbilitySpecHandle Handle, 
    const FGameplayAbilityActorInfo* ActorInfo,
    const FGameplayAbilityActivationInfo ActivationInfo, 
    bool bReplicateEndAbility, bool bWasCancelled)
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
    // UE_LOG(LogTemp, Log, TEXT("드롭킥 충돌 감지된 액터: %s"), *HitTarget->GetName());

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
        // UE_LOG(LogTemp, Warning, TEXT("1"));
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
        // UE_LOG(LogTemp, Log, TEXT("쥰내 센 드롭킥 파워: %f"), DropKickPower);
    }
    // Sound 재생
    if (BallKickSound)
    {
        UGameplayStatics::PlaySoundAtLocation(this, BallKickSound, BallActor->GetActorLocation());
    }
}

void UGA_SG_DropKick::ApplyDamageToTarget(AActor* HitEnemy, const FGameplayEventData& Payload)
{
    if (!HitEnemy || !DamageEffectClass)
    {
        return;
    }
    
    if (!IsOtherTeam(HitEnemy))
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
    
    // 무적상태 검사
    FGameplayTag ImmunityTag = FGameplayTag::RequestGameplayTag(FName("State.Immunity"));
    if (TargetASC->HasMatchingGameplayTag(ImmunityTag))
    {
        return;
    }
    
    FGameplayEffectContextHandle EffectContext = MyASC->MakeEffectContext();
    EffectContext.AddSourceObject(this);

    // 애님 몽타주 충돌 이벤트로부터 HitResult를 추출해 Context에 바인딩(Hit된 곳에 나이아가라 이펙트를 실행하기 위해)
    if (Payload.ContextHandle.IsValid())
    {
        const FHitResult* InsideHitResult = Payload.ContextHandle.GetHitResult();
        if (InsideHitResult)
        {
            EffectContext.AddHitResult(*InsideHitResult);
        }
    }

    FGameplayEffectSpecHandle NewHandle = MyASC->MakeOutgoingSpec(DamageEffectClass, 1.0f, EffectContext);
    if (NewHandle.IsValid())
    {
        FGameplayTag DropKickTag = FGameplayTag::RequestGameplayTag(FName("Character.Skill.DropKick"));
        NewHandle.Data.Get()->AddDynamicAssetTag(DropKickTag);
        
        MyASC->ApplyGameplayEffectSpecToTarget(*NewHandle.Data.Get(), TargetASC);
    }
}

bool UGA_SG_DropKick::IsOtherTeam(AActor* TargetActor) const
{
    if (CurrentActorInfo == nullptr || !CurrentActorInfo->AvatarActor.IsValid() || !IsValid(TargetActor))
    {
        return false;
    }
    
    APawn* OwnerPawn = Cast<APawn>(CurrentActorInfo->AvatarActor.Get());
    APawn* TargetPawn = Cast<APawn>(TargetActor);
    if (OwnerPawn == nullptr || TargetPawn == nullptr)
    {
        return false;
    }
    
    AController* OwnerController = OwnerPawn->GetController();
    AController* TargetController = TargetPawn->GetController();
    if (OwnerController == nullptr || TargetController == nullptr)
    {
        return false;
    }
    
    ASGMainPlayerState* OwnerPlayerState = OwnerController->GetPlayerState<ASGMainPlayerState>();
    ASGMainPlayerState* TargetPlayerState = TargetController->GetPlayerState<ASGMainPlayerState>();
    if (OwnerPlayerState == nullptr || TargetPlayerState == nullptr)
    {
        return false;
    }
    
    return OwnerPlayerState->CurrentTeamTag != TargetPlayerState->CurrentTeamTag;
}