// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/SceneComponent.h"
#include "SGRandomItemGrantVisualComponent.generated.h"


UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class SOCCERGAME_API USGRandomItemGrantVisualComponent : public USceneComponent
{
	GENERATED_BODY()

public:
	USGRandomItemGrantVisualComponent();

	virtual void BeginPlay() override;

	virtual void TickComponent(
		float DeltaTime,
		ELevelTick TickType,
		FActorComponentTickFunction* ThisTickFunction) override;

	UFUNCTION(BlueprintCallable, Category="Item|Visual")
	void SetVisualActive(bool bActive);
	
	UFUNCTION()
	USceneComponent* FindChildComponentByName(FName ComponentName) const;

private:

protected:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Item|Visual")
	TObjectPtr<USceneComponent> CubeRoot;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Item|Visual")
	TObjectPtr<USceneComponent> QuestionRoot;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Item|Visual")
	float RotationSpeed = 45.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Item|Visual")
	float BobAmplitude = 10.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Item|Visual")
	float BobSpeed = 2.f;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Item|Visual")
	FName CubeRootName = TEXT("CubeRoot");

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Item|Visual")
	FName QuestionRootName = TEXT("QuestionRoot");

private:
	FVector InitialCubeRootLocation = FVector::ZeroVector;
	FVector InitialQuestionRootLocation = FVector::ZeroVector;
	float RunningTime = 0.f;
		
};
