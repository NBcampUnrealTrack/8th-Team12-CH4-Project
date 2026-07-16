// ANS_SG_Kick.h

#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimNotifies/AnimNotifyState.h"
#include "ANS_SG_Kick.generated.h"

UCLASS()
class SOCCERGAME_API UANS_SG_Kick : public UAnimNotifyState
{
	GENERATED_BODY()
	
public:
	UANS_SG_Kick();
	
	virtual void NotifyBegin(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, float TotalDuration, const FAnimNotifyEventReference& EventReference) override;
	virtual void NotifyTick(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, float FrameDeltaTime, const FAnimNotifyEventReference& EventReference) override;
	
protected:
	// 발 부분 소켓 이름(Bone)
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Kick")
	FName SocketName;

	// 발차기 판정 구체의 반지름
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Kick")
	float TraceRadius;

	// 디버그 라인
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Kick")
	bool bDrawDebug;
	
private:
	// 중복 타격을 방지를 위해서 타격된 액터를 담는 배열
	UPROPERTY()
	TArray<TWeakObjectPtr<AActor>> HitActors;
};
