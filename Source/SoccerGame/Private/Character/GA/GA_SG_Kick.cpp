// GA_SG_Kick.cpp

#include "Character/GA/GA_SG_Kick.h"
#include "GameFramework/Character.h"
#include "Kismet/KismetSystemLibrary.h"
#include "AbilitySystemComponent.h"
#include "Character/GAS/GAS_SG_CharacterAttributeSet.h"
#include "Abilities/Tasks/AbilityTask_WaitInputRelease.h"
#include "Abilities/Tasks/AbilityTask_PlayMontageAndWait.h"
#include "Abilities/Tasks/AbilityTask_WaitGameplayEvent.h"

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
	UE_LOG(LogTemp, Warning, TEXT("발차기 차징 시작!"));
	
	// 차징 시간 계산
	if (GetWorld())
	{
		ChargeStartTime = GetWorld()->GetTimeSeconds();
	}
	
	// 마우스를 뗄 때까지 기다리는 GAS 내장 Task
	UAbilityTask_WaitInputRelease* WaitInputReleaseTask = UAbilityTask_WaitInputRelease::WaitInputRelease(this, false);
	if (WaitInputReleaseTask)
	{
		// 마우스를 떼면 OnInputReleased 함수가 실행되도록 바인딩
		WaitInputReleaseTask->OnRelease.AddDynamic(this, &UGA_SG_Kick::OnInputReleased);
		WaitInputReleaseTask->ReadyForActivation();
	}
	else
	{
		// 태스크 생성 실패 시 예외 처리 및 능력 즉시 종료
		// FindAndPushBall();
		EndAbility(Handle, ActorInfo, ActivationInfo, true, false);
	}
}

void UGA_SG_Kick::EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo,
	const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled)
{
	Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);
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
					// 밀어낼 방향 계산 (참고! PushDirection.Z 값이 클 수록 위로 뜹니다)
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

void UGA_SG_Kick::OnInputReleased(float TimeHeld)
{
	// 입력이 해제된 시점의 시간 구하기
	float CurrentTime = GetWorld() ? GetWorld()->GetTimeSeconds() : 0.0f;
	// 실제 누르고 있었던 시간 계산
	float ActualChargeTime = CurrentTime - ChargeStartTime;

	UE_LOG(LogTemp, Log, TEXT("마우스 입력 해제! 누른 시간: %f 초"), ActualChargeTime);
	
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
    
	// 최종 파워 저장
	CachedFinalKickPower = BaseKickPower * FinalMultiplier; 
	
	UAnimMontage* MontageToPlay = nullptr;

	// 0.5초(ActionSplitTime)보다 짧게 누르면 패스, 길게 누르면 슛
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

	// 캐릭터 몽타주 재생
	if (MontageToPlay)
	{
		// 단순히 몽타주만 재생하는 Task(델리게이트 바인딩 안 해도 됨)
		UAbilityTask_PlayMontageAndWait* PlayMontageTask = UAbilityTask_PlayMontageAndWait::CreatePlayMontageAndWaitProxy(
		   this,
		   NAME_None, 
		   MontageToPlay, 
		   1.0f, 
		   NAME_None, 
		   true
		);
		
		if (PlayMontageTask)
		{
			// 캔슬, 인터럽트(피격)등이 발생하면 어빌리티 종료
			PlayMontageTask->OnCompleted.AddDynamic(this, &UGA_SG_Kick::K2_EndAbility);
			PlayMontageTask->OnBlendOut.AddDynamic(this, &UGA_SG_Kick::K2_EndAbility);
			PlayMontageTask->OnInterrupted.AddDynamic(this, &UGA_SG_Kick::K2_EndAbility);
			PlayMontageTask->OnCancelled.AddDynamic(this, &UGA_SG_Kick::K2_EndAbility);
			
			PlayMontageTask->ReadyForActivation();
		}
		
		// 어빌리티 태그 정보가 private 일 수도 있어서 엔진이 제공하는 GetAssetTags()라는 함수를 사용하여 불러와야한다.
		FGameplayTagContainer AssetTagsContainer = GetAssetTags();
		FGameplayTag MyAbilityTag = FGameplayTag::EmptyTag;
		if (AssetTagsContainer.Num() > 0)
		{
			MyAbilityTag = AssetTagsContainer.GetByIndex(0);
		}

		// 태그 이벤트를 기다리는 Task
		UAbilityTask_WaitGameplayEvent* WaitEventTask = UAbilityTask_WaitGameplayEvent::WaitGameplayEvent(
		   this, 
		   MyAbilityTag,     // 에디터에서 설정한 GA_SG_Kick의 Tag
		   nullptr,          // Optional External Optional Actor
		   false             // bOnlyTriggerOnce (한 번만 트리거할지 여부)
		);

		if (WaitEventTask)
		{
			// 이벤트가 들어오면 실행할 함수 바인딩
			WaitEventTask->EventReceived.AddDynamic(this, &UGA_SG_Kick::OnGameplayEventReceived);
			WaitEventTask->ReadyForActivation();
		}
	}
	else
	{
		// 몽타주가 세팅이 안 되어 있다면 어빌리티 즉시 종료
		K2_EndAbility();
	}
}

void UGA_SG_Kick::OnGameplayEventReceived(FGameplayEventData Payload)
{
	UE_LOG(LogTemp, Log, TEXT("FindAndPushBall Test"));
	FindAndPushBall();
}