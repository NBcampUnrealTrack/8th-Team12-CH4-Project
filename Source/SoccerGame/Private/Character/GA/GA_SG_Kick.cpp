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

UGA_SG_Kick::UGA_SG_Kick()
{
	// 인스턴싱 정책 설정
	InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;
}

void UGA_SG_Kick::ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData)
{
	Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);

	// 능력 활성화가 가능 여부 체크
	if (!CommitAbility(Handle, ActorInfo, ActivationInfo))
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}
	
	// TEST!!
	FString NetMode = HasAuthority(&CurrentActivationInfo) ? TEXT("서버") : TEXT("클라이언트");
	UE_LOG(LogTemp, Warning, TEXT("[%s] 발차기 차징 시작!"), *NetMode);
	
	// 차징 시간 계산
	if (GetWorld())
	{
		ChargeStartTime = GetWorld()->GetTimeSeconds();
	}
	
	UAbilityTask_WaitInputRelease* WaitInputReleaseTask = UAbilityTask_WaitInputRelease::WaitInputRelease(this, true);
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
	Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);
}

void UGA_SG_Kick::InputReleased(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo)
{
    Super::InputReleased(Handle, ActorInfo, ActivationInfo);
	
    float CurrentTime = GetWorld() ? GetWorld()->GetTimeSeconds() : 0.0f;
    float ActualChargeTime = CurrentTime - ChargeStartTime;
	
	FString NetMode = HasAuthority(&CurrentActivationInfo) ? TEXT("서버") : TEXT("클라이언트");
    UE_LOG(LogTemp, Log, TEXT("[%s] 마우스 입력 해제, 누른 시간: %f 초"), *NetMode, ActualChargeTime);
    
    float BaseKickPower = 1000.0f;
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
    
    UAnimMontage* MontageToPlay = nullptr;

    if (ActualChargeTime < ActionSplitTime)
    {
       UE_LOG(LogTemp, Log, TEXT("숏 차징 (%f초) -> 패스 모션 재생"), ActualChargeTime);
       MontageToPlay = PassMontage;
    }
    else
    {
       UE_LOG(LogTemp, Log, TEXT("롱 차징 (%f초) -> 슛 모션 재생"), ActualChargeTime);
       MontageToPlay = ShootMontage;
    }

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
       FGameplayTag MyAbilityTag = FGameplayTag::EmptyTag;
       if (AssetTagsContainer.Num() > 0)
       {
          MyAbilityTag = AssetTagsContainer.GetByIndex(0);
       }

       UAbilityTask_WaitGameplayEvent* WaitEventTask = UAbilityTask_WaitGameplayEvent::WaitGameplayEvent(
          this, MyAbilityTag, nullptr, false
       );
       
       if (WaitEventTask)
       {
          WaitEventTask->EventReceived.AddDynamic(this, &UGA_SG_Kick::OnGameplayEventReceived);
          WaitEventTask->ReadyForActivation();
       }
       
       // 공 처리 레이더와 다르게 '데미지 주입 무전기'는 오직 주도권을 가진 서버만 개방합니다.
       // if (HasAuthority(&CurrentActivationInfo))
       // {
       //    // FGameplayTag HitTag = FGameplayTag::RequestGameplayTag(TEXT("Character.Skill.Kick.Hit"));
       //    UAbilityTask_WaitGameplayEvent* WaitHitEventTask = UAbilityTask_WaitGameplayEvent::WaitGameplayEvent(
       //       this, MyAbilityTag, nullptr, false             
       //    );
       //
       //    if (WaitHitEventTask)
       //    {
       //       WaitHitEventTask->EventReceived.AddDynamic(this, &UGA_SG_Kick::OnEnemyHitReceived);
       //       WaitHitEventTask->ReadyForActivation();
       //    }
       // }
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
		K2_EndAbility();
		return;
	}
	
	FVector Forward = Character->GetActorForwardVector();
	FVector StartLoc = Character->GetActorLocation() + (Forward * 50.0f);

	// 감지할 오브젝트 타입 설정 (물리 액터)
	TArray<TEnumAsByte<EObjectTypeQuery>> ObjectTypes;
	ObjectTypes.Add(UEngineTypes::ConvertToObjectType(ECollisionChannel::ECC_PhysicsBody));

	TArray<AActor*> ActorsToIgnore;
	ActorsToIgnore.Add(Character);

	TArray<AActor*> OutActors;

	// 가상의 구체(현재 반지름 100으로 설정)로 축구공 스캔
	bool bHit = UKismetSystemLibrary::SphereOverlapActors(
		GetWorld(), 
		StartLoc, 
		100.0f, 
		ObjectTypes, 
		nullptr, 
		ActorsToIgnore, 
		OutActors
	);

	if (bHit)
	{
		for (AActor* HitActor : OutActors)
		{
			// 축구공 찾기
			if (HitActor->GetName().Contains(TEXT("SoccerBall")) || HitActor->ActorHasTag(TEXT("Ball")))
			{
				UStaticMeshComponent* BallMesh = Cast<UStaticMeshComponent>(HitActor->GetRootComponent());
				if (BallMesh && BallMesh->IsSimulatingPhysics())
				{
					// 밀어낼 방향 계산 (PushDirection.Z 값이 클 수록 위로 뜬다)
					FVector PushDirection = (HitActor->GetActorLocation() - Character->GetActorLocation()).GetSafeNormal();
					PushDirection.Z += 1.2f; 
					PushDirection = PushDirection.GetSafeNormal();

					// AttributeSet의 KickPower를 사용하여 임펄스 설정
					BallMesh->AddImpulse(PushDirection * CachedFinalKickPower, NAME_None, true);
					UE_LOG(LogTemp, Log, TEXT("킥 파워: %f"), CachedFinalKickPower);
				}
			}
		}
	}
	
	K2_EndAbility();
}

void UGA_SG_Kick::OnGameplayEventReceived(FGameplayEventData Payload)
{
	if (!HasAuthority(&CurrentActivationInfo))
	{
		return;
	}
	
	UE_LOG(LogTemp, Log, TEXT("FindAndPushBall Test"));
	FindAndPushBall();
	
	OnEnemyHitReceived(Payload);
}

void UGA_SG_Kick::OnEnemyHitReceived(FGameplayEventData Payload)
{
	// 서버 체크
	if (!HasAuthority(&CurrentActivationInfo))
	{
		return;
	}

	// 타겟 액터 유효성 검사
	AActor* HitEnemy = const_cast<AActor*>(Payload.Target.Get());
	if (!HitEnemy || !DamageEffectClass)
	{
		return;
	}

	// 나의 어빌리티 시스템(ASC)과 적의 어빌리티 시스템(ASC)을 가져옴
	UAbilitySystemComponent* MyASC = GetAbilitySystemComponentFromActorInfo();
	// UAbilitySystemComponent* TargetASC = UAbilitySystemGlobals::GetAbilitySystemComponentFromActor(HitEnemy);
	UAbilitySystemComponent* TargetASC = nullptr;
	if (IAbilitySystemInterface* ASCInterface = Cast<IAbilitySystemInterface>(HitEnemy))
	{
		TargetASC = ASCInterface->GetAbilitySystemComponent();
	}
	else
	{
		TargetASC = UAbilitySystemGlobals::GetAbilitySystemComponentFromActor(HitEnemy);
	}

	if (!MyASC)
	{
		UE_LOG(LogTemp, Error, TEXT("🚨 [데미지 에러] 공격자(나)의 ASC를 찾을 수 없습니다."));
		return;
	}

	if (!TargetASC)
	{
		UE_LOG(LogTemp, Error, TEXT("🚨 [데미지 에러] 피격자(%s)의 ASC를 가져오지 못했습니다! 대상에게 ASC가 없거나 인터페이스가 빠졌을 수 있습니다."), *HitEnemy->GetName());
		return;
	}
	
	// 모든 컴포넌트가 유효할 때 실행
	UE_LOG(LogTemp, Warning, TEXT("🎯 [서버] 모든 조건 통과! %s 에게 데미지 주입 시도"), *HitEnemy->GetName());

	FGameplayEffectContextHandle EffectContext = MyASC->MakeEffectContext();
	EffectContext.AddSourceObject(this);

	FGameplayEffectSpecHandle NewHandle = MyASC->MakeOutgoingSpec(DamageEffectClass, 1.0f, EffectContext);
    
	if (NewHandle.IsValid())
	{
		// ⚔️ 실제 데미지 적용 및 결과 구조체 반환
		FActiveGameplayEffectHandle ActiveGEHandle = MyASC->ApplyGameplayEffectSpecToTarget(*NewHandle.Data.Get(), TargetASC);
        
		if (ActiveGEHandle.IsValid())
		{
			UE_LOG(LogTemp, Log, TEXT("⚔️ [서버] 발차기 데미지(GE) 가 성공적으로 타겟에 활성화 및 주입되었습니다!"));
		}
		else
		{
			UE_LOG(LogTemp, Error, TEXT("🚨 [데미지 에러] ApplyGameplayEffectSpecToTarget 함수가 실패했습니다! GE 에셋 내부 설정을 확인하세요."));
		}
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("🚨 [데미지 에러] GameplayEffectSpecHandle을 생성하지 못했습니다."));
	}
	
	// if (MyASC && TargetASC)
	// {
	// 	// -------------------------- 중요! --------------------------
	// 	// 데미지 테스트를 위해서 일단 주석처리, Tag 부여 후 같은팀, 적팀 테스트 예정
	// 	// FGameplayTag BlueTeamTag = FGameplayTag::RequestGameplayTag(TEXT("Team.Blue"));
	// 	// FGameplayTag RedTeamTag = FGameplayTag::RequestGameplayTag(TEXT("Team.Red"));
	// 	//
	// 	// // 내가 블루팀인지 레드팀인지 확인
	// 	// bool bIAmBlue = MyASC->HasMatchingGameplayTag(BlueTeamTag);
	// 	// bool bIAmRed = MyASC->HasMatchingGameplayTag(RedTeamTag);
	// 	//
	// 	// // 상대방이 블루팀인지 레드팀인지 확인
	// 	// bool bTargetIsBlue = TargetASC->HasMatchingGameplayTag(BlueTeamTag);
	// 	// bool bTargetIsRed = TargetASC->HasMatchingGameplayTag(RedTeamTag);
	// 	//
	// 	// // 둘의 팀이 완전히 일치하는 경우 (블루끼리 찼거나, 레드끼리 찼을 때)
	// 	// if ((bIAmBlue && bTargetIsBlue) || (bIAmRed && bTargetIsRed))
	// 	// {
	// 	// 	UE_LOG(LogTemp, Log, TEXT("같은 팀이라서 데미지 무효"));
	// 	// 	return; 
	// 	// }
	// 	//
	// 	// UE_LOG(LogTemp, Warning, TEXT("적팀에게 가하는 데미지 계산(서버)"));
	//
	// 	// 데미지 적용에 필요한 Context(인스티게이터 정보 등) 생성
	// 	FGameplayEffectContextHandle EffectContext = MyASC->MakeEffectContext();
	// 	EffectContext.AddSourceObject(this);
	//
	// 	// 에디터에서 선택한 GE 스펙 핸들 생성
	// 	FGameplayEffectSpecHandle NewHandle = MyASC->MakeOutgoingSpec(DamageEffectClass, 1.0f, EffectContext);
 //        
	// 	if (NewHandle.IsValid())
	// 	{
	// 		// 상대방 캐릭터에게 데미지를 준다.(Apply)
	// 		MyASC->ApplyGameplayEffectSpecToTarget(*NewHandle.Data.Get(), TargetASC);
 //            
	// 		UE_LOG(LogTemp, Log, TEXT("발차기 데미지 전달 완료"));
	// 	}
	// }
}