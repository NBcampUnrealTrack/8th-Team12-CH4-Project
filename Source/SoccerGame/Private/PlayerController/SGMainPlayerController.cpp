// Fill out your copyright notice in the Description page of Project Settings.


#include "PlayerController/SGMainPlayerController.h"

void ASGMainPlayerController::BeginPlay()
{
	Super::BeginPlay();
	ApplyGameInputMode();
	UE_LOG(LogTemp, Warning, TEXT("[MainPC] Main Game mode applied: %s / Pawn=%s"), *GetNameSafe(this), *GetNameSafe(GetPawn()));
}

void ASGMainPlayerController::OnPossess(APawn* InPawn)
{
	Super::OnPossess(InPawn);
	ApplyGameInputMode();
}

void ASGMainPlayerController::AcknowledgePossession(APawn* P)
{
	Super::AcknowledgePossession(P);

	UE_LOG(LogTemp, Warning,
		TEXT("[MainPC] AcknowledgePossession: %s / Pawn=%s / IsLocal=%d"),
		*GetNameSafe(this),
		*GetNameSafe(P),
		IsLocalController());

	ApplyGameInputMode();
}

void ASGMainPlayerController::ApplyGameInputMode()
{
	UE_LOG(LogTemp, Warning,
		TEXT("[MainPC] Input mode check: %s / IsLocal=%d / HasAuthority=%d / Pawn=%s"),
		*GetNameSafe(this),
		IsLocalController(),
		HasAuthority(),
		*GetNameSafe(GetPawn()));

	if (!IsLocalController())
	{
		return;
	}

	SetInputMode(FInputModeGameOnly());
	bShowMouseCursor = false;

	UE_LOG(LogTemp, Warning, TEXT("[MainPC] Local game input mode applied: %s / Pawn=%s"), *GetNameSafe(this), *GetNameSafe(GetPawn()));
}
