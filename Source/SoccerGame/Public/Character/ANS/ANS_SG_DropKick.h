// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimNotifies/AnimNotifyState.h"
#include "ANS_SG_DropKick.generated.h"

/**
 * 
 */
UCLASS()
class SOCCERGAME_API UANS_SG_DropKick : public UAnimNotifyState
{
	GENERATED_BODY()
	
public:
	UANS_SG_DropKick();
	
	virtual void NotifyBegin(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, float TotalDuration, const FAnimNotifyEventReference& EventReference) override;
	
	virtual void NotifyTick(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, float FrameDeltaTime, const FAnimNotifyEventReference& EventReference) override;
	
protected:
	// 발 부분 소켓 이름(Bone)
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "DropKick")
	FName SocketName;

	// 발차기 판정 구체의 반지름
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "DropKick")
	float TraceRadius;

	// 디버그 라인
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "DropKick")
	bool bDrawDebug;
	
private:
	// 중복 타격을 방지를 위해서 타격된 액터를 담는 배열
	UPROPERTY()
	TArray<TWeakObjectPtr<AActor>> HitActors;
};
