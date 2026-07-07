// SGRandomItemGrantVisualComponent.cpp

#include "Item/Visual/SGRandomItemGrantVisualComponent.h"

#include "Components/PrimitiveComponent.h"

USGRandomItemGrantVisualComponent::USGRandomItemGrantVisualComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
	PrimaryComponentTick.bStartWithTickEnabled = true;

	SetIsReplicatedByDefault(false);
}

void USGRandomItemGrantVisualComponent::BeginPlay()
{
	Super::BeginPlay();
	if (GetNetMode() == NM_DedicatedServer)
	{
		SetComponentTickEnabled(false);
		SetVisibility(false, true);
		return;
	}
	if (!IsValid(CubeRoot))
	{
		CubeRoot = FindChildComponentByName(CubeRootName);
	}

	if (!IsValid(QuestionRoot))
	{
		QuestionRoot = FindChildComponentByName(QuestionRootName);
	}

	if (IsValid(CubeRoot))
	{
		InitialCubeRootLocation = CubeRoot->GetRelativeLocation();
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("RandomItem visual CubeRoot not found."));
		SetComponentTickEnabled(false);
	}

	if (IsValid(QuestionRoot))
	{
		InitialQuestionRootLocation = QuestionRoot->GetRelativeLocation();
	}
}

void USGRandomItemGrantVisualComponent::TickComponent(
	float DeltaTime,
	ELevelTick TickType,
	FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	RunningTime += DeltaTime;

	if (IsValid(CubeRoot))
	{
		CubeRoot->AddLocalRotation(FRotator(0.f, RotationSpeed * DeltaTime, 0.f));

		const float BobOffset = FMath::Sin(RunningTime * BobSpeed) * BobAmplitude;
		CubeRoot->SetRelativeLocation(InitialCubeRootLocation + FVector(0.f, 0.f, BobOffset));
	}

	if (IsValid(QuestionRoot))
	{
		const float BobOffset = FMath::Sin(RunningTime * BobSpeed) * BobAmplitude;
		QuestionRoot->SetRelativeLocation(InitialQuestionRootLocation + FVector(0.f, 0.f, BobOffset));
	}
}

void USGRandomItemGrantVisualComponent::SetVisualActive(bool bActive)
{
	SetVisibility(bActive, true);
	SetComponentTickEnabled(bActive && GetNetMode() != NM_DedicatedServer);
}

USceneComponent* USGRandomItemGrantVisualComponent::FindChildComponentByName(FName ComponentName) const
{
	TArray<USceneComponent*> Children;
	GetChildrenComponents(true, Children);

	for (USceneComponent* Child : Children)
	{
		if (IsValid(Child) && Child->GetFName() == ComponentName)
		{
			return Child;
		}
	}

	return nullptr;
}
