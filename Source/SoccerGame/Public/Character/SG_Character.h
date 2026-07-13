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
DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FOnStaminaChanged, float, CurrentStamina, float, MaxStamina, float, StaminaPercent);

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
	
	// 기본 Ability를 부여하는 함수
	void GiveDefaultAbilities();
	
	// Item Slot Input
	void UseItemPressed();
	void UseItemReleased();
	
	// Item Rotation 입력
	void ItemRotation(const FInputActionValue& Value);
	
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
	
public:
	//-------------------------------- Ragdoll --------------------------------//
	// 래그돌 실행
	UFUNCTION(NetMulticast, Reliable)
	void MulticastEnableRagdoll(FVector HitImpulse, FVector HitLocation);

	void EnableRagdoll(FVector HitImpulse, FVector HitLocation);
	
	// 래그돌 해제
	UFUNCTION(NetMulticast, Reliable)
	void MulticastDisableRagdoll();

	void DisableRagdoll();

protected:
	// --- 래그돌 해제 후 일어나는 애니메이션 몽타주 ---
	// 엎드려서 일어나는 몽타주 (Face Down)
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Ragdoll|Animation")
	TObjectPtr<UAnimMontage> GetUpFrontMontage;

	// 뒤로 자빠져서 일어나는 몽타주 (Face Up)
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Ragdoll|Animation")
	TObjectPtr<UAnimMontage> GetUpBackMontage;

private:
	// 엎드려 있는지 확인하는 함수
	bool IsRagdollFaceDown() const;

	// 래그돌 해제 후 애니메이션 종료 시 이동 복구용 콜백
	UFUNCTION()
	void OnGetUpMontageEnded(UAnimMontage* Montage, bool bInterrupted);
	
protected:
	// SlotComponent
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "ItemSlot")
	TObjectPtr<USGItemSlotComponent> ItemSlotComponent;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input)
	UInputAction* UseItemAction;
	
	// Item Rotation 입력
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input)
	UInputAction* ItemRotationAction;
	
private:
	UPROPERTY()
	float BaseWalkSpeed;
	
	void OnSpeedMultiplierChanged(const FOnAttributeChangeData& Data);
	void ApplySpeedMultiplier(float NewMultiplier);
	
	//-------------------------------- 스태미나 UI --------------------------------//
public:
	// 스태미나 변경 시 블루프린트(UI)로 전달할 브로드캐스트 델리게이트
	UPROPERTY(BlueprintAssignable, Category = "UI|Stamina")
	FOnStaminaChanged OnStaminaChanged;

protected:
	// 스태미나 변경 감지 함수
	void OnStaminaAttributeChanged(const struct FOnAttributeChangeData& Data);
};
