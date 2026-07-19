// SG_Character.cpp

#include "SoccerGame/Public/Character/SG_Character.h"
#include "Engine/LocalPlayer.h"
#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "GameFramework/Controller.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "InputActionValue.h"

#include "Character/GAS/GAS_SG_CharacterAttributeSet.h"
#include "AbilitySystemComponent.h"
#include "Item/SGItemSlotComponent.h"
#include "PlayerState/SGMainPlayerState.h"

DEFINE_LOG_CATEGORY(Log_SG_Character);

// Sets default values
ASG_Character::ASG_Character()
{
 	
	GetCapsuleComponent()->InitCapsuleSize(42.f, 96.0f);
	
	bUseControllerRotationPitch = false;
	bUseControllerRotationYaw = false;
	bUseControllerRotationRoll = false;

	GetCharacterMovement()->bOrientRotationToMovement = true;	
	GetCharacterMovement()->RotationRate = FRotator(0.0f, 500.0f, 0.0f);
	
	GetCharacterMovement()->JumpZVelocity = 400.f;
	GetCharacterMovement()->AirControl = 0.25f;
	GetCharacterMovement()->MaxWalkSpeed = 500.f;
	GetCharacterMovement()->MinAnalogWalkSpeed = 20.f;
	GetCharacterMovement()->BrakingDecelerationWalking = 2000.f;
	GetCharacterMovement()->BrakingDecelerationFalling = 1500.0f;
	
	BaseWalkSpeed = GetCharacterMovement()->MaxWalkSpeed;
	
	CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
	CameraBoom->SetupAttachment(RootComponent);
	CameraBoom->TargetArmLength = 300.0f; 	
	CameraBoom->bUsePawnControlRotation = true;
	
	FollowCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FollowCamera"));
	FollowCamera->SetupAttachment(CameraBoom, USpringArmComponent::SocketName);
	FollowCamera->bUsePawnControlRotation = false;
	
	ItemSlotComponent = CreateDefaultSubobject<USGItemSlotComponent>(TEXT("ItemSlotComponent"));
	AbilitySystemComponent = CreateDefaultSubobject<UAbilitySystemComponent>(TEXT("AbilitySystemComponent"));
	AttributeSet = CreateDefaultSubobject<UGAS_SG_CharacterAttributeSet>(TEXT("GASAttributeSetBase"));
	// 네트워크 설정
	AbilitySystemComponent->SetIsReplicated(true); 
	AbilitySystemComponent->SetReplicationMode(EGameplayEffectReplicationMode::Mixed);
	bReplicates = true;
	SetReplicateMovement(true);
	GetMesh()->SetIsReplicated(true);
	
}

void ASG_Character::BeginPlay()
{
	Super::BeginPlay();
	
	if (HasAuthority() && StaminaRegenEffectClass) // 서버에서 적용하면 클라로 동기화
	{
		UAbilitySystemComponent* ASC = GetAbilitySystemComponent();
		if (ASC)
		{
			FGameplayEffectContextHandle EffectContext = ASC->MakeEffectContext();
			EffectContext.AddSourceObject(this);

			// 기본 스태미나 회복이라서 나 자신에게 적용
			FGameplayEffectSpecHandle NewHandle = ASC->MakeOutgoingSpec(StaminaRegenEffectClass, 1.0f, EffectContext);
			if (NewHandle.IsValid())
			{
				ASC->ApplyGameplayEffectSpecToSelf(*NewHandle.Data.Get());
			}
		}
	}
	
	if (AbilitySystemComponent)
	{
		// ASC 내부 초기화 함수 호출 (Owner와 Avatar 세팅)
		// AbilitySystemComponent->InitAbilityActorInfo(this, this);
		
		// 기본 능력 부여
		// GiveDefaultAbilities();
		
		// SpeedMultiplier 변경 사항 감지
		if (!AttributeSet)
		{
			return;
		}
		AbilitySystemComponent->GetGameplayAttributeValueChangeDelegate(
        			AttributeSet->GetStaminaAttribute()
        		).AddUObject(this, &ASG_Character::OnStaminaAttributeChanged);
		
		AbilitySystemComponent->GetGameplayAttributeValueChangeDelegate(
			AttributeSet->GetSpeedAttribute()).AddUObject(this, &ASG_Character::OnSpeedChanged);
		ApplySpeedChange(AttributeSet->GetSpeed());
	}
}

void ASG_Character::NotifyControllerChanged()
{
	Super::NotifyControllerChanged();
	
	if (APlayerController* PlayerController = Cast<APlayerController>(Controller))
	{
		if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(PlayerController->GetLocalPlayer()))
		{
			Subsystem->AddMappingContext(DefaultMappingContext, 0);
		}
	}
}

void ASG_Character::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	if (UEnhancedInputComponent* EnhancedInputComponent = Cast<UEnhancedInputComponent>(PlayerInputComponent)) {
		
		// Jumping
		EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Started, this, &ACharacter::Jump);
		EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Completed, this, &ACharacter::StopJumping);

		// Moving
		EnhancedInputComponent->BindAction(MoveAction, ETriggerEvent::Triggered, this, &ASG_Character::Move);

		// Looking
		EnhancedInputComponent->BindAction(LookAction, ETriggerEvent::Triggered, this, &ASG_Character::Look);
		
		// Use Item
		EnhancedInputComponent->BindAction(UseItemAction, ETriggerEvent::Started, this, &ASG_Character::UseItemPressed);
		EnhancedInputComponent->BindAction(UseItemAction, ETriggerEvent::Completed, this, &ASG_Character::UseItemReleased);

		EnhancedInputComponent->BindAction(ItemRotationAction, ETriggerEvent::Triggered, this, &ASG_Character::ItemRotation);
		
		// GAS + EnhancedInputComponent
		if (AbilitySystemComponent)
		{
			FTopLevelAssetPath AbilityInputBindsAssetPath = FTopLevelAssetPath(TEXT("/Script/SoccerGame"), TEXT("ESGAbilityInputID"));
			
			AbilitySystemComponent->BindAbilityActivationToInputComponent(EnhancedInputComponent, 
			FGameplayAbilityInputBinds(
				FString("Confirm"), 
				FString("Cancel"), 
				AbilityInputBindsAssetPath, 
				static_cast<int32>(ESGAbilityInputID::Confirm), 
				static_cast<int32>(ESGAbilityInputID::Cancel)
				)
			);
			
			if (IA_Kick)
			{
				// 누르는 순간 즉시 GAS에 Press 신호를 전달하여 어빌리티 실행
				EnhancedInputComponent->BindAction(IA_Kick, ETriggerEvent::Started, this, &ASG_Character::AbilityInputPressed, static_cast<int32>(ESGAbilityInputID::Kick));
			}
			
			if (IA_DropKick)
			{
				// 누르는 순간 즉시 GAS에 Press 신호를 전달하여 어빌리티 실행
				EnhancedInputComponent->BindAction(IA_DropKick, ETriggerEvent::Started, this, &ASG_Character::AbilityInputPressed, static_cast<int32>(ESGAbilityInputID::DropKick));
			}
		}
	}
	else
	{
		UE_LOG(Log_SG_Character, Error, TEXT("'%s' Failed to find an Enhanced Input component! This template is built to use the Enhanced Input system. If you intend to use the legacy system, then you will need to update this C++ file."), *GetNameSafe(this));
	}
}

void ASG_Character::InitializeDefaultAttributes()
{
	if (AttributeSet)
	{
		AttributeSet->InitMaxHp(CharacterMaxHp);
		AttributeSet->InitHp(CharacterMaxHp);
		AttributeSet->InitMaxStamina(CharacterMaxStamina);
		AttributeSet->InitStamina(CharacterMaxStamina);
		AttributeSet->InitKickPower(CharacterKickPower);
		AttributeSet->InitSpeed(CharacterSpeed);
	}
}

void ASG_Character::PossessedBy(AController* NewConroller)
{
	Super::PossessedBy(NewConroller);
	
	if (AbilitySystemComponent)
	{
		AbilitySystemComponent->InitAbilityActorInfo(this, this);
		InitializeDefaultAttributes();
		GiveDefaultAbilities(); 
	}
}

void ASG_Character::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

void ASG_Character::Move(const FInputActionValue& Value)
{
	if (AbilitySystemComponent && AbilitySystemComponent->HasMatchingGameplayTag(FGameplayTag::RequestGameplayTag(FName("State.Immunity"))))
	{
		// 무적 상태(HitReact 발동 혹은 래그돌 후 일어나는 경우)일 땐 움직이지 못하게 한다.
		return;
	}
	
	FVector2D MovementVector = Value.Get<FVector2D>();

	if (Controller != nullptr)
	{
		const FRotator Rotation = Controller->GetControlRotation();
		const FRotator YawRotation(0, Rotation.Yaw, 0);
		
		const FVector ForwardDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);
		const FVector RightDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);
		
		AddMovementInput(ForwardDirection, MovementVector.Y);
		AddMovementInput(RightDirection, MovementVector.X);
	}
}

void ASG_Character::Look(const FInputActionValue& Value)
{
	FVector2D LookAxisVector = Value.Get<FVector2D>();

	if (Controller != nullptr)
	{
		AddControllerYawInput(LookAxisVector.X);
		AddControllerPitchInput(LookAxisVector.Y);
	}
}

UAbilitySystemComponent* ASG_Character::GetAbilitySystemComponent() const
{
	return AbilitySystemComponent;
}

// 래핑 함수들 구현
void ASG_Character::AbilityInputPressed(int32 InputID)
{
	if (AbilitySystemComponent)
	{
		AbilitySystemComponent->AbilityLocalInputPressed(InputID);
	}
}

void ASG_Character::GiveDefaultAbilities()
{
	// GiveAbility는 멀티플레이 보안 상 반드시 서버(Authority)의 권한이여야 한다.
	if (!HasAuthority() || !AbilitySystemComponent)
	{
		return;
	}

	if (KickAbilityClass)
	{
		// 발차기 능력 생성
		FGameplayAbilitySpec KickSpec(KickAbilityClass);
		
		// 발차기 능력에 [Enum::Kick]을 매핑
		KickSpec.InputID = static_cast<int32>(ESGAbilityInputID::Kick);
		
		AbilitySystemComponent->GiveAbility(KickSpec);
	}
	
	if (DropKickAbilityClass)
	{
		// 드롭킥 능력 생성
		FGameplayAbilitySpec DropKickSpec(DropKickAbilityClass);
		
		// 드롭킥 능력에 [Enum::DropKick]을 매핑
		DropKickSpec.InputID = static_cast<int32>(ESGAbilityInputID::DropKick);
		
		AbilitySystemComponent->GiveAbility(DropKickSpec);
	}
	
	if (KickReactAbilityClass)
	{
		FGameplayAbilitySpec KickReactSpec(KickReactAbilityClass);
		
		AbilitySystemComponent->GiveAbility(KickReactSpec);
	}
}
	
void ASG_Character::UseItemPressed()
{
	if (!ItemSlotComponent)
	{
		return;
	}
	ItemSlotComponent->UseItemPressed();
}

void ASG_Character::UseItemReleased()
{
	if (!ItemSlotComponent)
	{
		return;
	}
	ItemSlotComponent->UseItemReleased();
}

void ASG_Character::ItemRotation(const FInputActionValue& Value)
{
	if (!ItemSlotComponent)
	{
		return;
	}
	const float InputValue = Value.Get<float>();
	ItemSlotComponent->UseItemRotate(InputValue);
}

void ASG_Character::OnRep_PlayerState()
{
	Super::OnRep_PlayerState();
	
	// 서버로부터 플레이어 정보가 복제되어 넘어왔을 때 세팅
	if (AbilitySystemComponent)
	{
		AbilitySystemComponent->InitAbilityActorInfo(this, this);
		InitializeDefaultAttributes();
		
		// PlayerState에서 팀 태그를 가져와 내 GAS 태그로 등록
		if (ASGMainPlayerState* TargetPS = GetPlayerState<ASGMainPlayerState>())
		{
			AbilitySystemComponent->AddLooseGameplayTag(TargetPS->CurrentTeamTag);
		}
	}
}

void ASG_Character::OnSpeedChanged(const FOnAttributeChangeData& Data)
{
	ApplySpeedChange(Data.NewValue);
}

void ASG_Character::ApplySpeedChange(float NewSpeed)
{
	GetCharacterMovement()->MaxWalkSpeed = NewSpeed;
}

void ASG_Character::OnStaminaAttributeChanged(const FOnAttributeChangeData& Data)
{
	float CurrentStamina = Data.NewValue;

	float MaxStamina = AbilitySystemComponent->GetNumericAttribute(
		UGAS_SG_CharacterAttributeSet::GetMaxStaminaAttribute()
	);

	float StaminaPercent = (MaxStamina > 0.0f) ? (CurrentStamina / MaxStamina) : 0.0f;

	OnStaminaChanged.Broadcast(CurrentStamina, MaxStamina, StaminaPercent);
}

// ---------------------------------------------------------------------------------------------- //
// --------------------------------------- Ragdoll System --------------------------------------- //
// ---------------------------------------------------------------------------------------------- //

void ASG_Character::MulticastEnableRagdoll_Implementation(FVector HitImpulse, FVector HitLocation)
{
	EnableRagdoll(HitImpulse, HitLocation);
}

void ASG_Character::EnableRagdoll(FVector HitImpulse, FVector HitLocation)
{
	USkeletalMeshComponent* MeshComp = GetMesh();
	UCapsuleComponent* CapsuleComp = GetCapsuleComponent();
	UCharacterMovementComponent* MovementComp = GetCharacterMovement();

	if (!MeshComp || !CapsuleComp || !MovementComp)
	{
		return;
	}
	
	if (AbilitySystemComponent)
	{
		FGameplayTag ImmunityTag = FGameplayTag::RequestGameplayTag(FName("State.Immunity"));
		AbilitySystemComponent->AddLooseGameplayTag(ImmunityTag);
	}

	// 캡슐 콜리전 비활성화
	CapsuleComp->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	// 캐릭터 이동 및 입력 멈춤
	MovementComp->DisableMovement();
	MovementComp->StopMovementImmediately();

	// 메쉬의 피직스 시뮬레이션 켜기
	MeshComp->SetCollisionProfileName(TEXT("Ragdoll"));
	MeshComp->SetAllBodiesSimulatePhysics(true);
	MeshComp->SetSimulatePhysics(true);
	MeshComp->WakeAllRigidBodies();
	
	// 감쇄 및 속도 부여
	MeshComp->SetLinearDamping(0.1f);
	MeshComp->SetPhysicsLinearVelocity(HitImpulse);
	
	// SpringArm을 Mesh의 골반(Hips) 소켓에 부착
	if (CameraBoom) 
	{
		CameraBoom->AttachToComponent(GetMesh(), FAttachmentTransformRules::SnapToTargetNotIncludingScale, TEXT("Hips"));
	}

	// 드롭킥 Impulse 전달(Hips(골반)을 중심으로)
	MeshComp->AddImpulseAtLocation(HitImpulse, HitLocation, TEXT("Hips"));
	
	if (HasAuthority())
	{
		FTimerHandle GetUpTimerHandle;
		GetWorldTimerManager().SetTimer(GetUpTimerHandle, this, &ASG_Character::ServerDisableRagdoll, 5.0f, false);
	}
}

void ASG_Character::ServerDisableRagdoll()
{
	if (!HasAuthority())
	{
		return;
	}

	USkeletalMeshComponent* MeshComp = GetMesh();
	UCapsuleComponent* CapsuleComp = GetCapsuleComponent();
	if (!MeshComp || !CapsuleComp)
	{
		return;
	}

	// 서버 기준 래그돌 Hips 위치 및 회전 수집
	FVector HipsLocation = MeshComp->GetSocketLocation(TEXT("Hips"));
	FRotator HipsRotation = MeshComp->GetSocketRotation(TEXT("Hips"));
	bool bIsFaceDown = IsRagdollFaceDown();
	
	// LineTrace로 정확한 지형 바닥점 찾기
	FVector Start = HipsLocation + FVector(0.0f, 0.0f, 50.0f);
	FVector End = HipsLocation - FVector(0.0f, 0.0f, 150.0f);
	
	FHitResult HitResult;
	FCollisionQueryParams QueryParams;
	QueryParams.AddIgnoredActor(this);

	// 동기화할 캡슐 위치 및 회전값 연산
	// FVector NewCapsuleLocation = HipsLocation;
	// NewCapsuleLocation.Z += CapsuleComp->GetScaledCapsuleHalfHeight();

	FVector NewCapsuleLocation = HipsLocation;
	if (GetWorld()->LineTraceSingleByChannel(HitResult, Start, End, ECC_Visibility, QueryParams))
	{
		NewCapsuleLocation.Z = HitResult.ImpactPoint.Z + CapsuleComp->GetScaledCapsuleHalfHeight();
	}
	else
	{
		NewCapsuleLocation.Z += CapsuleComp->GetScaledCapsuleHalfHeight();
	}
	
	FRotator NewCapsuleRotation = FRotator(0.0f, HipsRotation.Yaw, 0.0f);
	if (bIsFaceDown)
	{
		NewCapsuleRotation.Yaw += 180.0f;
	}

	// 계산된 완벽한 좌표 정보를 모든 클라이언트에 동기화
	MulticastDisableRagdoll(NewCapsuleLocation, NewCapsuleRotation, bIsFaceDown);
}

void ASG_Character::MulticastDisableRagdoll_Implementation(FVector TargetLocation, FRotator TargetRotation, bool bIsFaceDown)
{
	DisableRagdollInternal(TargetLocation, TargetRotation, bIsFaceDown);
}

void ASG_Character::DisableRagdollInternal(FVector TargetLocation, FRotator TargetRotation, bool bIsFaceDown)
{
    USkeletalMeshComponent* MeshComp = GetMesh();
    UCapsuleComponent* CapsuleComp = GetCapsuleComponent();
    UCharacterMovementComponent* MovementComp = GetCharacterMovement();

    if (!MeshComp || !CapsuleComp || !MovementComp)
    {
	    return;
    }
	
    // 시뮬레이션 끄기 직전 포즈 스냅샷 저장 (AnimBP 연결용)
    CacheRagdollPoseSnapshot();
	
	bIsRecoveringFromRagdoll = true;
	
	// 물리 및 속도 초기화
	MeshComp->SetAllPhysicsLinearVelocity(FVector::ZeroVector);
	MeshComp->SetAllPhysicsAngularVelocityInDegrees(FVector::ZeroVector);
	MeshComp->SetSimulatePhysics(false);
	MeshComp->SetCollisionProfileName(TEXT("CharacterMesh"));
	
	// 서버에서 결정된 동기화 위치로 캡슐 이동
	CapsuleComp->SetWorldLocation(TargetLocation, false, nullptr, ETeleportType::TeleportPhysics);
	CapsuleComp->SetWorldRotation(TargetRotation, false, nullptr, ETeleportType::TeleportPhysics);
	CapsuleComp->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	
	// 메쉬 상대 위치 오프셋 원복
	MeshComp->AttachToComponent(CapsuleComp, FAttachmentTransformRules::SnapToTargetNotIncludingScale);
	MeshComp->SetRelativeLocation(FVector(0.0f, 0.0f, -93.0f));
	MeshComp->SetRelativeRotation(FRotator(0.0f, -90.0f, 0.0f));
	
	// 카메라 SpringArm 원복
	if (CameraBoom)
	{
		CameraBoom->AttachToComponent(CapsuleComp, FAttachmentTransformRules::SnapToTargetNotIncludingScale);
		CameraBoom->SetRelativeLocation(FVector(0.0f, 0.0f, 0.0f));
	}
	
	// 방향에 맞는 GetUp 몽타주 재생
	UAnimMontage* TargetMontage = bIsFaceDown ? GetUpFrontMontage : GetUpBackMontage;
	UAnimInstance* AnimInst = MeshComp ? MeshComp->GetAnimInstance() : nullptr;
	if (TargetMontage && AnimInst)
	{
		const float PlayedDuration = AnimInst->Montage_Play(TargetMontage, 1.0f, EMontagePlayReturnType::MontageLength, 0.0f, true);
		
		if (PlayedDuration > 0.0f)
		{
			FOnMontageEnded EndedDelegate;
			EndedDelegate.BindUObject(this, &ASG_Character::OnGetUpMontageEnded);
			AnimInst->Montage_SetEndDelegate(EndedDelegate, TargetMontage);
		}
		else
		{
			// 몽타주 재생 실패시 예외 처리 (캐릭터 굳음 방지)
			UE_LOG(Log_SG_Character, Warning, TEXT("[%s] DisableRagdollInternal: Montage_Play Failed (Duration is 0.0)"), *GetName());
			bIsRecoveringFromRagdoll = false;
			MovementComp->SetMovementMode(EMovementMode::MOVE_Walking);
		}
	}
	else
	{
		// 몽타주가 없는 예외 경우 복구
		bIsRecoveringFromRagdoll = false;
		MovementComp->SetMovementMode(EMovementMode::MOVE_Walking);
	}
}

void ASG_Character::CacheRagdollPoseSnapshot()
{
	if (UAnimInstance* AnimInst = GetMesh() ? GetMesh()->GetAnimInstance() : nullptr)
	{
		AnimInst->SavePoseSnapshot(FName("RagdollFinalPose"));
	}
}

// 엎드려 있는지 판단 (골반 Up 벡터 기반)
bool ASG_Character::IsRagdollFaceDown() const
{
    USkeletalMeshComponent* MeshComp = GetMesh();
    if (!MeshComp)
    {
	    return true;
    }

    // 골반(Hips) 뼈의 회전 축을 이용해 바닥(Z축)을 바라보고 있는지 체크
    FVector HipsUpVector = MeshComp->GetSocketQuaternion(TEXT("Hips")).GetUpVector();
    
    // Z축 내적 결과가 음수이면 바닥을 바라보고 엎드린 상태 (Face Down)
    return FVector::DotProduct(HipsUpVector, FVector::UpVector) < 0.0f;
}

// 일어나기 애니메이션이 끝나면 조작 및 이동 가능 상태로 전환
void ASG_Character::OnGetUpMontageEnded(UAnimMontage* Montage, bool bInterrupted)
{
	bIsRecoveringFromRagdoll = false;
	
    if (UCharacterMovementComponent* MovementComp = GetCharacterMovement())
    {
        MovementComp->SetMovementMode(EMovementMode::MOVE_Walking);
    }
	
	if (AbilitySystemComponent)
	{
		FGameplayTag ImmunityTag = FGameplayTag::RequestGameplayTag(FName("State.Immunity"));
		AbilitySystemComponent->RemoveLooseGameplayTag(ImmunityTag);
		RecoveryHpRatio();
	}
}

void ASG_Character::RecoveryHpRatio()
{
	if (HasAuthority())
	{
		if (UGAS_SG_CharacterAttributeSet* SG_AttributeSet = const_cast<UGAS_SG_CharacterAttributeSet*>(AbilitySystemComponent->GetSet<UGAS_SG_CharacterAttributeSet>()))
		{
			float MaxHp = SG_AttributeSet->GetMaxHp();
			float RecoverHp = MaxHp * RagdollRecoveryHpRatio;

			SG_AttributeSet->SetHp(RecoverHp);
		}
	}
}

USoundBase* ASG_Character::GetRandomAttackVoiceSound() const
{
	if (AttackVoiceSounds.Num() == 0)
	{
		return nullptr;
	}
	
	int32 RandomIndex = FMath::RandRange(0, AttackVoiceSounds.Num() - 1);
	return AttackVoiceSounds[RandomIndex];
}

USoundBase* ASG_Character::GetRandomHitVoiceSound() const
{
	if (HitVoiceSounds.Num() == 0)
	{
		return nullptr;
	}
	int32 RandomIndex = FMath::RandRange(0, HitVoiceSounds.Num() - 1);
	return HitVoiceSounds[RandomIndex];
}