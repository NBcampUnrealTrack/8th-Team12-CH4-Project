// GAS_SG_CharacterAttributeSet.cpp


#include "Character/GAS/GAS_SG_CharacterAttributeSet.h"
#include "GameplayEffectExtension.h"
#include "Net/UnrealNetwork.h"
#include "NativeGameplayTags.h"
#include "Character/SG_Character.h"
#include "NiagaraFunctionLibrary.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "NiagaraComponent.h"

// 공격 종류 및 피격 태그 정의
UE_DEFINE_GAMEPLAY_TAG_STATIC(TAG_Attack_DropKick, "Character.Skill.DropKick");
UE_DEFINE_GAMEPLAY_TAG_STATIC(TAG_HitReact_Kick, "Character.HitReact.Kick");

UGAS_SG_CharacterAttributeSet::UGAS_SG_CharacterAttributeSet()
{
	// 속성 초기화
	InitHp(100.f);
	InitMaxHp(100.f);
	InitStamina(0.f);
	InitMaxStamina(100.f);
	InitKickPower(600.f);
	InitSpeed(500.f);
}

void UGAS_SG_CharacterAttributeSet::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	// REPNOTIFY_Always는 서버에서 값이 안 바뀌었어도 클라가 뒤늦게 접속하면 세팅해주도록 보장하는 GAS 표준 옵션
	DOREPLIFETIME_CONDITION_NOTIFY(UGAS_SG_CharacterAttributeSet, KickPower, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UGAS_SG_CharacterAttributeSet, Hp, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UGAS_SG_CharacterAttributeSet, MaxHp, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UGAS_SG_CharacterAttributeSet, Stamina, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UGAS_SG_CharacterAttributeSet, MaxStamina, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UGAS_SG_CharacterAttributeSet, Speed, COND_None, REPNOTIFY_Always);
}

// 변수 값을 바꿀때 사용할 함수들 (추후에 구체화 예정!)
bool UGAS_SG_CharacterAttributeSet::PreGameplayEffectExecute(struct FGameplayEffectModCallbackData& Data)
{
	Super::PreGameplayEffectExecute(Data);
	
	// GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Green, TEXT("PreGameplayEffectExecute"));
	
	return true;
}

void UGAS_SG_CharacterAttributeSet::PreAttributeChange(const FGameplayAttribute& Attribute, float& NewValue)
{
	Super::PreAttributeChange(Attribute, NewValue);
	
	if (Attribute == GetHpAttribute())
	{
		NewValue = FMath::Clamp(NewValue, 0.0f, GetMaxHp());
	}
	else if (Attribute == GetStaminaAttribute())
	{
		NewValue = FMath::Clamp(NewValue, 0.f, GetMaxStamina());
	}
	
	// GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Green, TEXT("PreAttributeChange"));
}

void UGAS_SG_CharacterAttributeSet::PostAttributeChange(const FGameplayAttribute& Attribute, float OldValue,
	float NewValue)
{
	Super::PostAttributeChange(Attribute, OldValue, NewValue);
	
	// 변경된 어트리뷰트가 Speed
	if (Attribute == GetSpeedAttribute())
	{
		// 소유하고 있는 캐릭터를 가져옴
		ACharacter* OwningCharacter = Cast<ACharacter>(GetOwningActor());
		if (OwningCharacter)
		{
			// 캐릭터 무브먼트 컴포넌트의 MaxWalkSpeed를 직접 750(NewValue)으로 변경
			UCharacterMovementComponent* MoveComp = OwningCharacter->GetCharacterMovement();
			if (MoveComp)
			{
				MoveComp->MaxWalkSpeed = NewValue;
			}
		}
	}
	
	// GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Green, TEXT("PostAttributeChange"));
}

void UGAS_SG_CharacterAttributeSet::PostGameplayEffectExecute(const struct FGameplayEffectModCallbackData& Data)
{
	Super::PostGameplayEffectExecute(Data);
	
	if (Data.EvaluatedData.Attribute == GetHpAttribute())
	{
		// SetHp는 BaseHp를 Set해준다.
		SetHp(GetHp());
	}
	else if (Data.EvaluatedData.Attribute == GetStaminaAttribute())
	{
		SetStamina(GetStamina());
	}
	
	// Damage 메타 속성이 들어왔을 때
	if (Data.EvaluatedData.Attribute == GetDamageAttribute())
	{
		const float LocalDamageDone = GetDamage();
		SetDamage(0.f); // 계산 완료 후 초기화
		
		AActor* TargetActor = Data.Target.GetAvatarActor();
		FGameplayCueParameters CueParams;
		if (TargetActor)
		{
			CueParams.Location = TargetActor->GetActorLocation();
			CueParams.Instigator = Data.EffectSpec.GetEffectContext().GetEffectCauser();
			CueParams.EffectCauser = Data.EffectSpec.GetEffectContext().GetEffectCauser();
			CueParams.TargetAttachComponent = TargetActor->GetRootComponent(); 
		}
		Data.Target.ExecuteGameplayCue(FGameplayTag::RequestGameplayTag(FName("GameplayCue.Character.Voice.Hit")), CueParams);

		if (LocalDamageDone > 0.f)
		{
			// 현재 HP 감산
			const float NewHp = FMath::Clamp(GetHp() - LocalDamageDone, 0.f, GetMaxHp());
			SetHp(NewHp);
			
			// GE 스펙 내의 모든 Asset Tag 및 Captured Tag 긁어옴
			FGameplayTagContainer CombinedTags;
			// GE 블루프린트 Asset Tags
			Data.EffectSpec.GetAllAssetTags(CombinedTags);
			// Captured Source Tags
			if (const FGameplayTagContainer* SourceTags = Data.EffectSpec.CapturedSourceTags.GetAggregatedTags())
			{
				CombinedTags.AppendTags(*SourceTags);
			}
			// 디버그
			// UE_LOG(LogTemp, Warning, TEXT("감지된 전체 태그 목록: %s"), *CombinedTags.ToString());

			// GE의 TAG로 식별
			const FGameplayTag DropKickTag = FGameplayTag::RequestGameplayTag(FName("Character.Skill.DropKick"));
			const bool bIsDropKick = CombinedTags.HasTag(DropKickTag);
			
			if (bIsDropKick)
			{
				// Context에서 Hit Result 긁어오기
				FGameplayEffectContextHandle Context = Data.EffectSpec.GetEffectContext();
				const FHitResult* HitResult = Context.GetHitResult();
        
				FVector SpawnLocation;
				FRotator SpawnRotation = FRotator::ZeroRotator;

				if (HitResult && HitResult->bBlockingHit)
				{
					SpawnLocation = HitResult->ImpactPoint;
					// 타격면의 법선 벡터를 기준으로 이펙트 방향 설정
					SpawnRotation = HitResult->ImpactNormal.Rotation();
				}
				else
				{
					// HitResult가 없다면 타겟 캐릭터의 위치(골반쯤)로 대체
					SpawnLocation = TargetActor ? TargetActor->GetActorLocation() : FVector::ZeroVector;
				}
				
				if (ASG_Character* TargetCharacter = Cast<ASG_Character>(TargetActor))
				{
					UNiagaraSystem* DropKickEffect = TargetCharacter->GetDropKickEffect();
            
					if (DropKickEffect)
					{
						UNiagaraFunctionLibrary::SpawnSystemAtLocation(
							GetWorld(), 
							DropKickEffect, 
							SpawnLocation, 
							SpawnRotation
						);
					}
				}
			}
			
			// HP가 0 이하(그로기)
			if (GetHp() <= 0.f)
			{
				TargetActor = Data.Target.GetAvatarActor();
				ASG_Character* TargetCharacter = Cast<ASG_Character>(TargetActor);
				
				if (bIsDropKick)
				{
					// 드롭킥으로 HP가 0 -> 래그돌
					if (TargetCharacter)
					{
						
						AActor* Attacker = Data.EffectSpec.GetEffectContext().GetEffectCauser();
						FVector LaunchDirection;
						if (Attacker)
						{
							LaunchDirection = (TargetActor->GetActorLocation() - Attacker->GetActorLocation()).GetSafeNormal();
						}
						else
						{
							LaunchDirection = TargetActor->GetActorForwardVector() * -1.0f;
						}
						
						float UpwardRatio = TargetCharacter->GetRagdollUpwardForceRatio();
						float PowerMultiplier = TargetCharacter->GetRagdollKickPowerMultiplier();
						
						LaunchDirection.Z += UpwardRatio; 
						LaunchDirection = LaunchDirection.GetSafeNormal();
						
						float BaseDropKickPower = 2500.0f;
						FVector HitImpulse = LaunchDirection * (BaseDropKickPower * PowerMultiplier);
						FVector HitLocation = TargetActor->GetActorLocation();
                        
						TargetCharacter->MulticastEnableRagdoll(HitImpulse, TargetActor->GetActorLocation());
						
						Data.Target.ExecuteGameplayCue(FGameplayTag::RequestGameplayTag(FName("GameplayCue.Character.Hit.DropKick.Death")), CueParams);
					}
				}
				else
				{
					Data.Target.ExecuteGameplayCue(FGameplayTag::RequestGameplayTag(FName("GameplayCue.Character.Hit.Kick")), CueParams);
					FGameplayEventData Payload;
					Payload.Instigator = Data.EffectSpec.GetEffectContext().GetEffectCauser();
					Payload.Target = TargetActor;

					UAbilitySystemComponent* TargetASC = &Data.Target;
					if (TargetASC)
					{
						TargetASC->HandleGameplayEvent(TAG_HitReact_Kick, &Payload);
                    
						FGameplayTagContainer AbilityTags;
						AbilityTags.AddTag(TAG_HitReact_Kick);
						TargetASC->TryActivateAbilitiesByTag(AbilityTags);
					}
				}
			}
			else
			{
				if (bIsDropKick)
				{
					Data.Target.ExecuteGameplayCue(FGameplayTag::RequestGameplayTag(FName("GameplayCue.Character.Hit.DropKick")), CueParams);
				}
				else
				{
					Data.Target.ExecuteGameplayCue(FGameplayTag::RequestGameplayTag(FName("GameplayCue.Character.Hit.Kick")), CueParams);
				}
			}
		}
	}
	
	// GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Green, TEXT("PostGameplayEffectExecute"));
}

// GAMEPLAYATTRIBUTE_REPNOTIFY 매크로를 사용해 내부 예측(Prediction) 시스템과 연동
void UGAS_SG_CharacterAttributeSet::OnRep_KickPower(const FGameplayAttributeData& OldKickPower)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UGAS_SG_CharacterAttributeSet, KickPower, OldKickPower);
}

void UGAS_SG_CharacterAttributeSet::OnRep_Speed(const FGameplayAttributeData& OldSpeed)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UGAS_SG_CharacterAttributeSet, Speed, OldSpeed);
}

void UGAS_SG_CharacterAttributeSet::OnRep_Hp(const FGameplayAttributeData& OldHp)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UGAS_SG_CharacterAttributeSet, Hp, OldHp);
}

void UGAS_SG_CharacterAttributeSet::OnRep_MaxHp(const FGameplayAttributeData& OldMaxHp)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UGAS_SG_CharacterAttributeSet, MaxHp, OldMaxHp);
}

void UGAS_SG_CharacterAttributeSet::OnRep_Stamina(const FGameplayAttributeData& OldStamina)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UGAS_SG_CharacterAttributeSet, Stamina, OldStamina);
}

void UGAS_SG_CharacterAttributeSet::OnRep_MaxStamina(const FGameplayAttributeData& OldMaxStamina)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UGAS_SG_CharacterAttributeSet, MaxStamina, OldMaxStamina);
}