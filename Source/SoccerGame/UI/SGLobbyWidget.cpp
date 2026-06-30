// Fill out your copyright notice in the Description page of Project Settings.


#include "SGLobbyWidget.h"

void USGLobbyWidget::NativeConstruct()
{
	Super::NativeConstruct();
	
	// TODO: 테스트용 플레이어 데이터 배열 구성 위치
	// TODO: RefreshLobby 호출 위치
}

void USGLobbyWidget::RefreshLobby()
{
	// TODO: PlayerInfos에 InPlayerInfos 대입
	// TODO: RefreshLobby 호출
}

void USGLobbyWidget::SetPlayerInfos(const TArray<FSGPlayerLobbyInfo>& InPlayerInfos)
{
	// TODO: VerticalBox_BlueTeam 유효성 검사
	// TODO: VerticalBox_RedTeam 유효성 검사
	// TODO: VerticalBox_Waiting 유효성 검사
	// TODO: PlayerSlotWidgetClass 유효성 검사
	
	// TODO: 기존 Blue 팀 슬롯 제거
	// TODO: 기존 Red 팀 슬롯 제거
	// TODO: 기존 Waiting 팀 슬롯 제거
	
	for (const FSGPlayerLobbyInfo& PlayerInfo : PlayerInfos)
	{
		// TODO: PlayerSlotWidgetClass 기반 위젯 생성 변수 선언
		// TODO: 생성된 슬롯 위젯 유효성 검사
		// TODO: SetPlayerSlotInfo 호출 위치
		// TODO: TeamType에 따라 추가할 VerticalBox 분기 위치
	}
}
