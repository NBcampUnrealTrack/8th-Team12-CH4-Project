// Fill out your copyright notice in the Description page of Project Settings.


#include "GameState/SGMainGameState.h"
#include "Net/UnrealNetwork.h"

ASGMainGameState::ASGMainGameState()
{
}

void ASGMainGameState::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	
		
	DOREPLIFETIME(ASGMainGameState, CurrentMatchState);
	DOREPLIFETIME(ASGMainGameState, RedTeamScore);
	DOREPLIFETIME(ASGMainGameState, BlueTeamScore);
	DOREPLIFETIME(ASGMainGameState, CurrentGameTime);
}

void ASGMainGameState::OnRep_MatchState()
{
}

void ASGMainGameState::OnRep_RedTeamScore()
{
}

void ASGMainGameState::OnRep_BlueTeamScore()
{
}
