// Fill out your copyright notice in the Description page of Project Settings.


#include "GameState/SGLobbyGameState.h"
#include "Net/UnrealNetwork.h"

ASGLobbyGameState::ASGLobbyGameState()
{
	// 이 액터가 네트워크를 통해 복제되도록 활성화
	bReplicates = true;
}

void ASGLobbyGameState::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(ASGLobbyGameState, bIsReady);
	UE_LOG(LogTemp, Log, TEXT("Clinet Checking "));
}

void ASGLobbyGameState::OnRep_IsReady()
{
	// [클라이언트] 준비 상태 변경에 따른 UI 토글 로직을 여기에 연동
	UE_LOG(LogTemp, Log, TEXT("[Client] Synced Ready Status: %s"), bIsReady ? TEXT("TRUE") : TEXT("FALSE"));
}
