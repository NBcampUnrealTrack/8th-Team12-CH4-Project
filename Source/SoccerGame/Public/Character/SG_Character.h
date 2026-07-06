// SG_Character.h

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "Logging/LogMacros.h"
#include "AbilitySystemInterface.h"
#include "AbilitySystemComponent.h"
#include "SG_Character.generated.h"

class USGItemSlotComponent;
class USpringArmComponent;
class UCameraComponent;
class UInputMappingContext;
class UInputAction;
struct FInputActionValue;

DECLARE_LOG_CATEGORY_EXTERN(Log_SG_Character, Log, All);

//-------------------------------- AbilityInputID를 설정하는 ENUM --------------------------------//
UENUM(BlueprintType)
enum class ESGAbilityInputID : uint8
{
	// --- 기본 항목 ---
	None			UMETA(DisplayName = "None"),
	Confirm			UMETA(DisplayName = "Confirm"),
	Cancel			UMETA(DisplayName = "Cancel"),
	
	// --- 추가 항목 ---
	Kick			UMETA(DisplayName = "Kick"),
	DropKick		UMETA(DisplayName = "DropKick")
};

UCLASS()
class SOCCERGAME_API ASG_Character : public ACharacter, public IAbilitySystemInterface
{
	GENERATED_BODY()
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
	USpringArmComponent* CameraBoom;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
	UCameraComponent* FollowCamera;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	UInputMappingContext* DefaultMappingContext;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	UInputAction* JumpAction;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	UInputAction* MoveAction;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	UInputAction* LookAction;
	
public:
	ASG_Character();
	
	FORCEINLINE class USpringArmComponent* GetCameraBoom() const { return CameraBoom; }
	FORCEINLINE class UCameraComponent* GetFollowCamera() const { return FollowCamera; }
	
	// Attribute Set
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AbilitySystem")
	TObjectPtr<class UGAS_SG_CharacterAttributeSet> AttributeSet;
	
	virtual UAbilitySystemComponent* GetAbilitySystemComponent() const override;
	
protected:
	virtual void BeginPlay() override;
	
	void Move(const FInputActionValue& Value);
	void Look(const FInputActionValue& Value);
	
	virtual void NotifyControllerChanged() override;
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;
	
	virtual void PossessedBy(AController* NewConroller) override;
	
private:	
	virtual void Tick(float DeltaTime) override;
	
	//-------------------------------- Ability Input  --------------------------------//
	void AbilityInputPressed(int32 InputID);
	void AbilityInputReleased(int32 InputID);
	
	// 기본 Ability를 부여하는 함수
	void GiveDefaultAbilities();
	
	// Item Slot Input
	void UseItemPressed();
	void UseItemReleased();
	
protected:
	//-------------------------------- Kick --------------------------------//
	// ASC 세팅
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "GAS")
	TObjectPtr<UAbilitySystemComponent> AbilitySystemComponent;
	
	// Kick 버튼
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
	class UInputAction* IA_Kick;
	
	// 에디터에서 할당할 발차기 GA 클래스 타입
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "GAS")
	TSubclassOf<class UGameplayAbility> KickAbilityClass;
	
	//-------------------------------- Drop Kick --------------------------------//
	// DropKick 버튼
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
	class UInputAction* IA_DropKick;
	
	// 에디터에서 할당할 발차기 GA 클래스 타입
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "GAS")
	TSubclassOf<class UGameplayAbility> DropKickAbilityClass;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "GAS|Passive")
	TSubclassOf<UGameplayEffect> StaminaRegenEffectClass;
	
	// 클라이언트에서 폰이 플레이어 스테이트를 리플리케이션 받았을 때 호출되는 함수
	virtual void OnRep_PlayerState() override;
	
protected:
	// SlotComponent
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "ItemSlot")
	TObjectPtr<USGItemSlotComponent> ItemSlotComponent;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input)
	UInputAction* UseItemAction;
	
private:
	UPROPERTY()
	float BaseWalkSpeed;
	
	void OnSpeedMultiplierChanged(const FOnAttributeChangeData& Data);
	void ApplySpeedMultiplier(float NewMultiplier);
};
