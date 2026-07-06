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
	
	AbilitySystemComponent = CreateDefaultSubobject<UAbilitySystemComponent>(TEXT("AbilitySystemComponent"));
	// 네트워크 설정
	AbilitySystemComponent->SetIsReplicated(true); 
	AbilitySystemComponent->SetReplicationMode(EGameplayEffectReplicationMode::Mixed);
	bReplicates = true;
	SetReplicateMovement(true);
	GetMesh()->SetIsReplicated(true);
	
	AttributeSet = CreateDefaultSubobject<UGAS_SG_CharacterAttributeSet>(TEXT("GASAttributeSetBase"));
	// CreateDefaultSubobject<UGAS_SG_CharacterAttributeSet>(TEXT("AttributeSet"));
	
	ItemSlotComponent = CreateDefaultSubobject<USGItemSlotComponent>(TEXT("ItemSlotComponent"));
}

void ASG_Character::BeginPlay()
{
	Super::BeginPlay();
	
	if (AbilitySystemComponent)
	{
		// ASC 내부 초기화 함수 호출 (Owner와 Avatar 세팅)
		// AbilitySystemComponent->InitAbilityActorInfo(this, this);
		
		// 기본 능력 부여
		// GiveDefaultAbilities();
		
		// SpeedMultiplier 변경 사항 감지
		if (!AttributeSet) return;
		AbilitySystemComponent->GetGameplayAttributeValueChangeDelegate(
			AttributeSet->GetSpeedMultiplierAttribute()).AddUObject(this, &ASG_Character::OnSpeedMultiplierChanged);
		ApplySpeedMultiplier(AttributeSet->GetSpeedMultiplier());
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
				// 누르는순간 GAS에 Press 신호 전달
				EnhancedInputComponent->BindAction(IA_Kick, ETriggerEvent::Started, this, &ASG_Character::AbilityInputPressed, static_cast<int32>(ESGAbilityInputID::Kick));
            
				// 떼는 순간 GAS에 Release 신호 전달 (WaitInputRelease Task가 이 신호를 감지)
				EnhancedInputComponent->BindAction(IA_Kick, ETriggerEvent::Completed, this, &ASG_Character::AbilityInputReleased, static_cast<int32>(ESGAbilityInputID::Kick));
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

void ASG_Character::PossessedBy(AController* NewConroller)
{
	Super::PossessedBy(NewConroller);
	
	if (AbilitySystemComponent)
	{
		AbilitySystemComponent->InitAbilityActorInfo(this, this);
		GiveDefaultAbilities(); 
	}
}

void ASG_Character::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

void ASG_Character::Move(const FInputActionValue& Value)
{
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

void ASG_Character::AbilityInputReleased(int32 InputID)
{
	if (AbilitySystemComponent)
	{
		AbilitySystemComponent->AbilityLocalInputReleased(InputID);
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

void ASG_Character::OnRep_PlayerState()
{
	Super::OnRep_PlayerState();
	
	// 서버로부터 플레이어 정보가 복제되어 넘어왔을 때 세팅
	if (AbilitySystemComponent)
	{
		AbilitySystemComponent->InitAbilityActorInfo(this, this);
		
		// PlayerState에서 팀 태그를 가져와 내 GAS 태그로 등록
		if (ASGMainPlayerState* TargetPS = GetPlayerState<ASGMainPlayerState>())
		{
			AbilitySystemComponent->AddLooseGameplayTag(TargetPS->CurrentTeamTag);
		}
	}
}

void ASG_Character::OnSpeedMultiplierChanged(const FOnAttributeChangeData& Data)
{
	ApplySpeedMultiplier(Data.NewValue);
}

void ASG_Character::ApplySpeedMultiplier(float NewMultiplier)
{
	GetCharacterMovement()->MaxWalkSpeed = BaseWalkSpeed * NewMultiplier;
}
