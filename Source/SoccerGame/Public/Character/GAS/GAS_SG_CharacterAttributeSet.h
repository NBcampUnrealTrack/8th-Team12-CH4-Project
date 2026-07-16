// GAS_SG_CharacterAttributeSet.h

#pragma once

#include "CoreMinimal.h"
#include "AttributeSet.h"
#include "AbilitySystemComponent.h"
#include "GAS_SG_CharacterAttributeSet.generated.h"

#define ATTRIBUTE_ACCESSORS(ClassName, PropertyName) \
GAMEPLAYATTRIBUTE_PROPERTY_GETTER(ClassName, PropertyName) \
GAMEPLAYATTRIBUTE_VALUE_GETTER(PropertyName) \
GAMEPLAYATTRIBUTE_VALUE_SETTER(PropertyName) \
GAMEPLAYATTRIBUTE_VALUE_INITTER(PropertyName)

UCLASS()
class SOCCERGAME_API UGAS_SG_CharacterAttributeSet : public UAttributeSet
{
	GENERATED_BODY()
	
public:
	UGAS_SG_CharacterAttributeSet();
	
	// 발차기 위력 속성
	UPROPERTY(BlueprintReadOnly, Category = "Attributes", ReplicatedUsing = OnRep_KickPower)
	FGameplayAttributeData KickPower;
	ATTRIBUTE_ACCESSORS(UGAS_SG_CharacterAttributeSet, KickPower);
	
	// 스피드 증가 배율 속성
	UPROPERTY(BlueprintReadOnly, Category = "Attributes", ReplicatedUsing = OnRep_SpeedMultiplier)
	FGameplayAttributeData SpeedMultiplier;
	ATTRIBUTE_ACCESSORS(UGAS_SG_CharacterAttributeSet, SpeedMultiplier);
	
	// 캐릭터 기본 속성
	UPROPERTY(BlueprintReadOnly, Category = "Attributes", ReplicatedUsing = OnRep_Hp)
	FGameplayAttributeData Hp;
	ATTRIBUTE_ACCESSORS(UGAS_SG_CharacterAttributeSet, Hp);

	UPROPERTY(BlueprintReadOnly, Category = "Attributes", ReplicatedUsing = OnRep_MaxHp)
	FGameplayAttributeData MaxHp;
	ATTRIBUTE_ACCESSORS(UGAS_SG_CharacterAttributeSet, MaxHp);

	UPROPERTY(BlueprintReadOnly, Category = "Attributes", ReplicatedUsing = OnRep_Stamina)
	FGameplayAttributeData Stamina;
	ATTRIBUTE_ACCESSORS(UGAS_SG_CharacterAttributeSet, Stamina);

	UPROPERTY(BlueprintReadOnly, Category = "Attributes", ReplicatedUsing = OnRep_MaxStamina)
	FGameplayAttributeData MaxStamina;
	ATTRIBUTE_ACCESSORS(UGAS_SG_CharacterAttributeSet, MaxStamina);
	
	// 데미지 계산 전용 메타 속성, 복제X
	UPROPERTY(BlueprintReadOnly, Category = "Attributes")
	FGameplayAttributeData Damage;
	ATTRIBUTE_ACCESSORS(UGAS_SG_CharacterAttributeSet, Damage);
	
	// 변수 값을 바꿀때 사용할 함수들
	virtual bool PreGameplayEffectExecute(struct FGameplayEffectModCallbackData &Data) override;
	virtual void PreAttributeChange(const FGameplayAttribute& Attribute, float& NewValue) override;
	virtual void PostAttributeChange(const FGameplayAttribute& Attribute, float OldValue, float NewValue) override;
	virtual void PostGameplayEffectExecute(const struct FGameplayEffectModCallbackData &Data) override;
	
protected:
	// 콜백 함수
	UFUNCTION()
	virtual void OnRep_KickPower(const FGameplayAttributeData& OldKickPower);
	UFUNCTION()
	virtual void OnRep_SpeedMultiplier(const FGameplayAttributeData& OldSpeedMultiplier);
	UFUNCTION() 
	virtual void OnRep_Hp(const FGameplayAttributeData& OldHp);
	UFUNCTION() 
	virtual void OnRep_MaxHp(const FGameplayAttributeData& OldMaxHp);
	UFUNCTION() 
	virtual void OnRep_Stamina(const FGameplayAttributeData& OldStamina);
	UFUNCTION() 
	virtual void OnRep_MaxStamina(const FGameplayAttributeData& OldMaxStamina);
};
