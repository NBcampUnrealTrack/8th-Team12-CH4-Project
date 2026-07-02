// GAS_SG_CharacterAttributeSet.cpp


#include "Character/GAS/GAS_SG_CharacterAttributeSet.h"
#include "Net/UnrealNetwork.h"

UGAS_SG_CharacterAttributeSet::UGAS_SG_CharacterAttributeSet()
{
	// 속성 초기화
	InitHp(100.f);
	InitMaxHp(100.f);
	InitStamina(100.f);
	InitMaxStamina(100.f);
	InitKickPower(2000.f);
	InitSpeedMultiplier(1.f);
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
}

// 변수 값을 바꿀때 사용할 함수들 (추후에 구체화 예정!)
bool UGAS_SG_CharacterAttributeSet::PreGameplayEffectExecute(struct FGameplayEffectModCallbackData& Data)
{
	Super::PreGameplayEffectExecute(Data);
	
	GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Green, TEXT("PreGameplayEffectExecute"));
	
	return true;
}

void UGAS_SG_CharacterAttributeSet::PreAttributeChange(const FGameplayAttribute& Attribute, float& NewValue)
{
	Super::PreAttributeChange(Attribute, NewValue);
	
	GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Green, TEXT("PreAttributeChange"));
}

void UGAS_SG_CharacterAttributeSet::PostAttributeChange(const FGameplayAttribute& Attribute, float OldValue,
	float NewValue)
{
	Super::PostAttributeChange(Attribute, OldValue, NewValue);
	
	GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Green, TEXT("PostAttributeChange"));
}

void UGAS_SG_CharacterAttributeSet::PostGameplayEffectExecute(const struct FGameplayEffectModCallbackData& Data)
{
	Super::PostGameplayEffectExecute(Data);
	
	GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Green, TEXT("PostGameplayEffectExecute"));
}

// GAMEPLAYATTRIBUTE_REPNOTIFY 매크로를 사용해 내부 예측(Prediction) 시스템과 연동
void UGAS_SG_CharacterAttributeSet::OnRep_KickPower(const FGameplayAttributeData& OldKickPower)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UGAS_SG_CharacterAttributeSet, KickPower, OldKickPower);
}

void UGAS_SG_CharacterAttributeSet::OnRep_SpeedMultiplier(const FGameplayAttributeData& OldSpeedMultiplier)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UGAS_SG_CharacterAttributeSet, SpeedMultiplier, OldSpeedMultiplier);
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