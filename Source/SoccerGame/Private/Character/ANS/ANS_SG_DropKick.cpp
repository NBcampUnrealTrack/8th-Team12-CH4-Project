// Fill out your copyright notice in the Description page of Project Settings.


#include "Character/ANS/ANS_SG_DropKick.h"
#include "Components/SkeletalMeshComponent.h"
#include "Kismet/KismetSystemLibrary.h"
#include "AbilitySystemBlueprintLibrary.h"
#include "GameplayTagContainer.h"
#include "AbilitySystemComponent.h"
#include "AbilitySystemInterface.h"

UANS_SG_DropKick::UANS_SG_DropKick()
{
	SocketName = TEXT("LeftToeSocket");
	TraceRadius = 10.0f;
	bDrawDebug = true;
}

void UANS_SG_DropKick::NotifyBegin(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, float TotalDuration,
	const FAnimNotifyEventReference& EventReference)
{
	Super::NotifyBegin(MeshComp, Animation, TotalDuration, EventReference);
	
	HitActors.Empty();
}

void UANS_SG_DropKick::NotifyTick(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, float FrameDeltaTime,
	const FAnimNotifyEventReference& EventReference)
{
	Super::NotifyTick(MeshComp, Animation, FrameDeltaTime, EventReference);
	
	if (!MeshComp || !MeshComp->GetOwner())
	{
		return;
	}

	AActor* MyCharacter = MeshComp->GetOwner();
    
	if (MyCharacter->GetWorld() && MyCharacter->GetWorld()->WorldType == EWorldType::EditorPreview)
	{
		return;
	}
	
	APawn* MyPawn = Cast<APawn>(MyCharacter);
	if (MyPawn)
	{
		// 내가 조종하는 것도 아니고, 서버 권한도 없다면 패스!
		// 즉, 서버 컴퓨터거나 혹은 내 화면(로컬)일 때만 이 아래 트레이스 로직을 통과합니다.
		if (!MyPawn->IsLocallyControlled() && !MyPawn->HasAuthority())
		{
			return;
		}
	}
    
	UWorld* World = MyCharacter->GetWorld();
	if (!World)
	{
		return;
	}
    
	FVector SocketLocation = MeshComp->GetSocketLocation(SocketName);
	TArray<TEnumAsByte<EObjectTypeQuery>> ObjectTypes;
    
	// Pawn(플레이어/AI) 타입 감지
	ObjectTypes.Add(UEngineTypes::ConvertToObjectType(ECollisionChannel::ECC_Pawn)); 
	
	// 축구공(물리 액터) 감지를 위해 PhysicsBody 추가
	ObjectTypes.Add(UEngineTypes::ConvertToObjectType(ECollisionChannel::ECC_PhysicsBody)); 

	// 일반 동적 오브젝트 채널도 추가(보험용)
	// ObjectTypes.Add(UEngineTypes::ConvertToObjectType(ECollisionChannel::ECC_WorldDynamic));

	TArray<AActor*> ActorsToIgnore;
	ActorsToIgnore.Add(MyCharacter);
    
	for (const auto& WeakActor : HitActors)
	{
		if (WeakActor.IsValid())
		{
			ActorsToIgnore.Add(WeakActor.Get());
		}
	}

	FHitResult HitResult;
	EDrawDebugTrace::Type DebugType = bDrawDebug ? EDrawDebugTrace::ForDuration : EDrawDebugTrace::None;
    
	bool bHit = UKismetSystemLibrary::SphereTraceSingleForObjects(
		World,
		SocketLocation,
		SocketLocation,
		TraceRadius,
		ObjectTypes,
		false,
		ActorsToIgnore,
		DebugType,
		HitResult,
		true,
		FLinearColor::Green,      
		FLinearColor::Red,    
		1.0f
	);

	// 충돌이 감지되었을 때 GAS Gameplay Event 전송
	if (bHit && HitResult.GetActor())
	{
		AActor* HitEnemy = HitResult.GetActor();
        
		// ANS 구간이 끝날 때까지 연타 버그 방지
		HitActors.Add(HitEnemy);

		// GAS 이벤트 전송용 데이터(Payload)
		FGameplayEventData Payload;
		Payload.Instigator = MyCharacter; // 공격자
		Payload.Target = HitEnemy;        // 피격자
		
		// 공격자의 ASC에서 Context를 만들고, 트레이스로 검출된 HitResult를 넣어준다.(Hit된 곳에 나이아가라 이펙트를 실행하기 위해)
		if (IAbilitySystemInterface* ASCInterface = Cast<IAbilitySystemInterface>(MyCharacter))
		{
			if (UAbilitySystemComponent* MyASC = ASCInterface->GetAbilitySystemComponent())
			{
				FGameplayEffectContextHandle ContextHandle = MyASC->MakeEffectContext();
				ContextHandle.AddHitResult(HitResult); 
				Payload.ContextHandle = ContextHandle;  
			}
		}

		// GAS 내장 라이브러리를 통해 캐릭터 어빌리티 시스템에 무전 송신
		FGameplayTag HitTag = FGameplayTag::RequestGameplayTag(TEXT("Character.Skill.DropKick"));
		UAbilitySystemBlueprintLibrary::SendGameplayEventToActor(MyCharacter, HitTag, Payload);
	}
}
