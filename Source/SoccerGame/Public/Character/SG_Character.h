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
class UNiagaraSystem;

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
	
	//-------------------------------- Character Stats --------------------------------//
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "GAS|Stats")
	float CharacterMaxHp = 100.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "GAS|Stats")
	float CharacterMaxStamina = 100.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "GAS|Stats")
	float CharacterKickPower = 600.0f;

	// 스탯 초기화 함수
	void InitializeDefaultAttributes();
	
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
	
	// 에디터에서 할당할 Kick React GA 클래스 타입
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "GAS")
	TSubclassOf<class UGameplayAbility> KickReactAbilityClass;
	
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
	// 서버에서 래그돌 해제 타이머 종료 시 호출
	void ServerDisableRagdoll();

	// 서버가 계산한 위치/회전/방향을 모든 클라이언트에 멀티캐스트
	UFUNCTION(NetMulticast, Reliable)
	void MulticastDisableRagdoll(FVector TargetLocation, FRotator TargetRotation, bool bIsFaceDown);

protected:
	// 실제 래그돌 해제 및 복구 로직
	void DisableRagdollInternal(FVector TargetLocation, FRotator TargetRotation, bool bIsFaceDown);

	// 물리 끄기 직전 포즈 캡처
	void CacheRagdollPoseSnapshot();
	
	// --- 래그돌 해제 후 일어나는 애니메이션 몽타주 ---
	// 엎드려서 일어나는 몽타주 (Face Down)
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Ragdoll|Animation")
	TObjectPtr<UAnimMontage> GetUpFrontMontage;

	// 뒤로 자빠져서 일어나는 몽타주 (Face Up)
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Ragdoll|Animation")
	TObjectPtr<UAnimMontage> GetUpBackMontage;

	// Ragdoll 복구 상태(ABP에서 사용)
	UPROPERTY(BlueprintReadOnly, Category = "Ragdoll")
	bool bIsRecoveringFromRagdoll = false;
	
private:
	// 엎드려 있는지 확인하는 함수
	bool IsRagdollFaceDown() const;

	// 래그돌 해제 후 애니메이션 종료 시 이동 복구용 콜백
	UFUNCTION()
	void OnGetUpMontageEnded(UAnimMontage* Montage, bool bInterrupted);
	
protected:
	// 드롭킥 피격 시 공중으로 떠오르는 비율 (BP에서 조절)
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Ragdoll|Stats")
	float RagdollUpwardForceRatio = 1.0f;

	// 드롭킥 피격 시 날아가는 힘 배율 (BP에서 조절)
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Ragdoll|Stats")
	float RagdollKickPowerMultiplier = 4.0f;
	
	// 래그돌 복구 후 회복할 HP 비율 (0.3f = 30% 회복, 1.0f = 풀피 회복, BP에서 조절)
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Ragdoll|Stats")
	float RagdollRecoveryHpRatio = 1.0f;
	
public:
	UFUNCTION()
	void RecoveryHpRatio();
	
	FORCEINLINE float GetRagdollUpwardForceRatio() const { return RagdollUpwardForceRatio; }
	FORCEINLINE float GetRagdollKickPowerMultiplier() const { return RagdollKickPowerMultiplier; }
	
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
	
protected:
	//-------------------------------- 나이아가라 --------------------------------//
	// 나이아가라 에셋
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Effects")
	TObjectPtr<UNiagaraSystem> DropKickEffect;

public:
	FORCEINLINE UNiagaraSystem* GetDropKickEffect() const { return DropKickEffect; }
	
	//-------------------------------- 스태미나 UI --------------------------------//
public:
	// 스태미나 변경 시 블루프린트(UI)로 전달할 브로드캐스트 델리게이트
	UPROPERTY(BlueprintAssignable, Category = "UI|Stamina")
	FOnStaminaChanged OnStaminaChanged;

protected:
	// 스태미나 변경 감지 함수
	void OnStaminaAttributeChanged(const struct FOnAttributeChangeData& Data);
	
	
public:
	//-------------------------------- Sound --------------------------------//
	// 랜덤 Attack Sound 가져오기
	UFUNCTION(BlueprintCallable, Category = "Sound")
	USoundBase* GetRandomAttackVoiceSound() const;
	
	// 랜덤 Hurt(Hit) Sound 가져오기
	UFUNCTION(BlueprintCallable, Category = "Sound")
	USoundBase* GetRandomHitVoiceSound() const;
	
protected:
	// Attack Sound
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Sound|Voice")
	TArray<TObjectPtr<USoundBase>> AttackVoiceSounds;

	// Hurt(Hit) Sound
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Sound|Voice")
	TArray<TObjectPtr<USoundBase>> HitVoiceSounds;
};
