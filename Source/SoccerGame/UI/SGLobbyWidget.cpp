// Fill out your copyright notice in the Description page of Project Settings.


#include "SGLobbyWidget.h"
#include "Components/Button.h"
#include "Components/TextBlock.h"
#include "Components/VerticalBox.h"

void USGLobbyWidget::NativeConstruct()
{
	Super::NativeConstruct();
	
	if (ReadyButton)
	{
		ReadyButton->OnClicked.AddDynamic(this, &USGLobbyWidget::OnReadyButtonClicked);
	}
	
	// TODO: 테스트 플레이어 데이터 배열 초기화 위치
	
	FSGPlayerLobbyInfo Player1;
	Player1.UserName = FText::FromString("Player1");
	FSGPlayerLobbyInfo Player2;
	Player2.UserName = FText::FromString("Player2");
	FSGPlayerLobbyInfo Player3;
	Player3.UserName = FText::FromString("Player3");
	FSGPlayerLobbyInfo Player4;
	Player4.UserName = FText::FromString("Player4");
	FSGPlayerLobbyInfo Player5;
	Player5.UserName = FText::FromString("Player5");
	FSGPlayerLobbyInfo Player6;
	Player6.UserName = FText::FromString("Player6");
	
	PlayerInfos.Add(Player1);
	PlayerInfos.Add(Player2);
	PlayerInfos.Add(Player3);
	PlayerInfos.Add(Player4);
	PlayerInfos.Add(Player5);
	PlayerInfos.Add(Player6);
	
	// TODO: RefreshLobby 호출 위치
	RefreshLobby();
	
	// TODO: UpdateReadyButtonText 호출 위치
	UpdateReadyButtonText();
}

void USGLobbyWidget::SetPlayerInfos(const TArray<FSGPlayerLobbyInfo>& InPlayerInfos)
{
	// TODO: PlayerInfos에 InPlayerInfos 대입
	PlayerInfos = InPlayerInfos;
	
	// TODO: RefreshLobby 호출
	RefreshLobby();
}

void USGLobbyWidget::RefreshLobby()
{
	// TODO: VerticalBox_BlueTeam 유효성 검사
	// TODO: VerticalBox_RedTeam 유효성 검사
	// TODO: VerticalBox_Waiting 유효성 검사
	if (!VerticalBox_BlueTeam || !VerticalBox_RedTeam || !VerticalBox_Waiting)
	{
		return;
	}
	
	// TODO: PlayerSlotWidgetClass 유효성 검사
	if (!PlayerSlotWidgetClass) return;
	
	// TODO: 기존 Blue 팀 슬롯 제거
	// TODO: 기존 Red 팀 슬롯 제거
	// TODO: 기존 Waiting 팀 슬롯 제거
	VerticalBox_BlueTeam->ClearChildren();
	VerticalBox_RedTeam->ClearChildren();
	VerticalBox_Waiting->ClearChildren();
	
	for (const FSGPlayerLobbyInfo& PlayerInfo : PlayerInfos)
	{
		// TODO: PlayerSlotWidgetClass 기반 위젯 생성 변수 선언
		for (const FSGPlayerLobbyInfo& PlayerInfo : PlayerInfos)
		{
			USGPlayerSlotWidget* PlayerSlotWidget = CreateWidget<USGPlayerSlotWidget>(GetWorld(), PlayerSlotWidgetClass);
			
			// TODO: 생성된 슬롯 위젯 유효성 검사
			if (!PlayerSlotWidget)
			{
				continue;
			}
			
			// TODO: SetPlayerSlotInfo 호출 위치
			SetPlayerSlotInfo(USGPlayerSlotWidget::StaticClass(), PlayerInfo, PlayerSlotWidget);
			// TODO: TeamType에 따라 추가할 VerticalBox 분기 위치
		}
		
	}
}



void USGLobbyWidget::OnReadyButtonClicked()
{
	// TODO: LocalPlayerIndex 유효성 검사
	// TODO: PlayerInfos[LocalPlayerIndex].bIsReady 값 토
	// TODO: RefreshLobby 호출
	// TODO: UpdateReadyButtonText 호출
}

void USGLobbyWidget::UpdateReadyButtonText()
{
	// TODO: Text_ReadyButton 유효성 검사
	// TODO: LocalPlayerIndex 유효성 검사
	// TODO: 현재 Ready 상태에 따른 버튼 텍스트 값
}
