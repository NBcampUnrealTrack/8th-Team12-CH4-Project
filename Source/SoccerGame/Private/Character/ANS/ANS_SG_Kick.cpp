// ANS_SG_Kick.cpp

#include "Character/ANS/ANS_SG_Kick.h"
#include "Components/SkeletalMeshComponent.h"
#include "Kismet/KismetSystemLibrary.h"
#include "AbilitySystemBlueprintLibrary.h"
#include "GameplayTagContainer.h"

UANS_SG_Kick::UANS_SG_Kick()
{
    SocketName = TEXT("RightToeSocket");
    TraceRadius = 50.0f;
    bDrawDebug = true;
}

void UANS_SG_Kick::NotifyBegin(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, float TotalDuration, const FAnimNotifyEventReference& EventReference)
{
    Super::NotifyBegin(MeshComp, Animation, TotalDuration, EventReference);

    // 발차기를 새로 할 때마다 이전 프레임들의 타격 리스트 리셋
    HitActors.Empty();
}

void UANS_SG_Kick::NotifyTick(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, float FrameDeltaTime, const FAnimNotifyEventReference& EventReference)
{
    Super::NotifyTick(MeshComp, Animation, FrameDeltaTime, EventReference);

    if (!MeshComp || !MeshComp->GetOwner())
    {
        return;
    }

    AActor* MyCharacter = MeshComp->GetOwner();
    UWorld* World = MyCharacter->GetWorld();
    if (!World)
    {
        return;
    }
    
    FVector SocketLocation = MeshComp->GetSocketLocation(SocketName);
    TArray<TEnumAsByte<EObjectTypeQuery>> ObjectTypes;
    
    // Pawn(플레이어/AI) 타입 감지
    ObjectTypes.Add(UEngineTypes::ConvertToObjectType(ECollisionChannel::ECC_Pawn)); 

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

        // GAS 내장 라이브러리를 통해 캐릭터 어빌리티 시스템에 무전 송신
        FGameplayTag HitTag = FGameplayTag::RequestGameplayTag(TEXT("Character.Skill.Kick.Hit"));
        UAbilitySystemBlueprintLibrary::SendGameplayEventToActor(MyCharacter, HitTag, Payload);
    }
}