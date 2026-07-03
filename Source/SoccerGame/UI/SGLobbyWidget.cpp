// Fill out your copyright notice in the Description page of Project Settings.


#include "SGLobbyWidget.h"
#include "SGPlayerSlotWidget.h"
#include "Components/Button.h"
#include "Components/TextBlock.h"
#include "Components/VerticalBox.h"

#include "SoccerGame/Public/PlayerController/SGLobbyPlayerController.h"

void USGLobbyWidget::NativeConstruct()
{
	Super::NativeConstruct();
	
	BlueTeamSlots = {BlueSlot_1, BlueSlot_2, BlueSlot_3};
	RedTeamSlots = {RedSlot_1, RedSlot_2, RedSlot_3};
	WaitingSlots = {WaitingSlot_1, WaitingSlot_2, WaitingSlot_3, WaitingSlot_4, WaitingSlot_5, WaitingSlot_6};
	
	FGameplayTag BlueTag = FGameplayTag::RequestGameplayTag(FName("Team.Blue"));
	FGameplayTag RedTag = FGameplayTag::RequestGameplayTag(FName("Team.Red"));
	FGameplayTag WaitingTag = FGameplayTag::RequestGameplayTag(FName("Team.Waiting"));
	
	// 슬롯에게 태그 부여, 클릭 알림 구독
	for (auto* BlueSlot : BlueTeamSlots)
	{
		if (BlueSlot)
		{
			BlueSlot->SetSlotTeamTag(BlueTag);
			BlueSlot->OnSlotClicked.AddDynamic(this, &USGLobbyWidget::HandleSlotClicked);
		}
	}
	for (auto* RedSlot : RedTeamSlots)
	{
		if (RedSlot)
		{
			RedSlot->SetSlotTeamTag(RedTag);
			RedSlot->OnSlotClicked.AddDynamic(this, &USGLobbyWidget::HandleSlotClicked);
		}
	}
	for (auto* WaitingSlot : WaitingSlots)
	{
		if (WaitingSlot)
		{
			WaitingSlot->SetSlotTeamTag(WaitingTag);
			WaitingSlot->OnSlotClicked.AddDynamic(this, &USGLobbyWidget::HandleSlotClicked);
		}
	}
	
	if (ReadyButton)
	{
		ReadyButton->OnClicked.AddDynamic(this, &USGLobbyWidget::OnReadyButtonClicked);
	}
	
	/* 테스트 플레이어 데이터 배열 초기화 위치 */
	PlayerInfos.Empty();
	/*
	// 블루팀
	FSGPlayerLobbyInfo Player1 = {};
	Player1.UserName = FText::FromString("Player1");
	Player1.TeamTag = WaitingTag;
	
	FSGPlayerLobbyInfo Player2 = {};
	Player2.UserName = FText::FromString("Player2");
	Player2.TeamTag = WaitingTag;
	
	FSGPlayerLobbyInfo Player3 = {};
	Player3.UserName = FText::FromString("Player3");
	Player3.TeamTag = WaitingTag;
	
	// 레드팀
	FSGPlayerLobbyInfo Player4 = {};
	Player4.UserName = FText::FromString("Player4");
	Player4.TeamTag = WaitingTag;
	
	FSGPlayerLobbyInfo Player5 = {};
	Player5.UserName = FText::FromString("Player5");
	Player5.TeamTag = WaitingTag;
	
	FSGPlayerLobbyInfo Player6 = {};
	Player6.UserName = FText::FromString("Player6");
	Player6.TeamTag = WaitingTag;
	
	PlayerInfos.Add(Player1);
	PlayerInfos.Add(Player2);
	PlayerInfos.Add(Player3);
	PlayerInfos.Add(Player4);
	PlayerInfos.Add(Player5);
	PlayerInfos.Add(Player6);
	
	RefreshLobby();
	UpdateReadyButtonText();
	*/
}

void USGLobbyWidget::SetPlayerInfos(const TArray<FSGPlayerLobbyInfo>& InPlayerInfos)
{
	PlayerInfos = InPlayerInfos;
	
	RefreshLobby();
	UpdateReadyButtonText();
}

void USGLobbyWidget::RefreshLobby()
{
	
	for (auto* BlueSlot : BlueTeamSlots)
	{
		if (BlueSlot) BlueSlot->ResetSlot();
	}
	for (auto* RedSlot : RedTeamSlots)
	{
		if (RedSlot) RedSlot->ResetSlot();
	}
	for (auto* WaitingSlot : WaitingSlots)
	{
		if (WaitingSlot) WaitingSlot->ResetSlot();
	}
		
	int CurrentBlueIndex = 0;
	int CurrentRedIndex = 0;
	int CurrentWaitingIndex = 0;
	
	FGameplayTag BlueTag = FGameplayTag::RequestGameplayTag(FName("Team.Blue"));
	FGameplayTag RedTag = FGameplayTag::RequestGameplayTag(FName("Team.Red"));
	FGameplayTag WaitingTag = FGameplayTag::RequestGameplayTag(FName("Team.Waiting"));
	
	for (const FSGPlayerLobbyInfo& PlayerInfo : PlayerInfos)
	{
		if (PlayerInfo.TeamTag == BlueTag)
		{
			// 블루팀 자리 3개 이하일 때만 데이터 세팅
			if (CurrentBlueIndex < 3 && BlueTeamSlots[CurrentBlueIndex])
			{
				BlueTeamSlots[CurrentBlueIndex]->SetPlayerSlotInfo(PlayerInfo.UserName, PlayerInfo.bIsReady, PlayerInfo.TeamTag);
				CurrentBlueIndex++;
			}
		}
		else if (PlayerInfo.TeamTag == RedTag)
		{
			if (CurrentRedIndex < 3 && RedTeamSlots[CurrentRedIndex])
			{
				RedTeamSlots[CurrentRedIndex]->SetPlayerSlotInfo(PlayerInfo.UserName, PlayerInfo.bIsReady, PlayerInfo.TeamTag);
				CurrentRedIndex++;
			}
		}
		else if (PlayerInfo.TeamTag == WaitingTag)
		{
			if (CurrentWaitingIndex < 6 && WaitingSlots[CurrentWaitingIndex])
			{
				WaitingSlots[CurrentWaitingIndex]->SetPlayerSlotInfo(PlayerInfo.UserName, PlayerInfo.bIsReady, PlayerInfo.TeamTag);
				CurrentWaitingIndex++;
			}
		}
	}
}


void USGLobbyWidget::OnReadyButtonClicked()
{
	// LocalPlayerIndex 유효성 검사
	if (!PlayerInfos.IsValidIndex(LocalPlayerIndex)) return;
	
	// PlayerInfos[LocalPlayerIndex].bIsReady 값 토글
	PlayerInfos[LocalPlayerIndex].bIsReady = !PlayerInfos[LocalPlayerIndex].bIsReady;
	
	//  이 위젯을 소유한 로컬 PlayerController 가져오기
	ASGLobbyPlayerController* LobbyPC = Cast<ASGLobbyPlayerController>(GetOwningPlayer());
	if (LobbyPC)
	{
		//  내 컨트롤러를 통해 서버에 레디 상태 토글 요청전달
		LobbyPC->SellectReady();
	}
	
	RefreshLobby();
	UpdateReadyButtonText();
}

void USGLobbyWidget::UpdateReadyButtonText()
{

	if (!Text_ReadyButton) return;
	if (!PlayerInfos.IsValidIndex(LocalPlayerIndex)) return;
	if (PlayerInfos[LocalPlayerIndex].bIsReady == false)
	{
		Text_ReadyButton->SetText(FText::FromString("Ready"));	
	}
	else
	{
		Text_ReadyButton->SetText(FText::FromString("Cancel"));
	}
}

void USGLobbyWidget::HandleSlotClicked(FGameplayTag RequestedTeamTag)
{
	// 내 정보가 유효한지 확인
	if (!PlayerInfos.IsValidIndex(LocalPlayerIndex)) return;
	
	if (PlayerInfos[LocalPlayerIndex].TeamTag == RequestedTeamTag) return;
	
	// 위젯을 소유한 플레이어 컨트롤러 가져오기
	ASGLobbyPlayerController* LobbyPC = Cast<ASGLobbyPlayerController>(GetOwningPlayer());
	if (LobbyPC)
	{
		// 내 컨트롤러를 통해 서버에 팀 변경 요청 전달
		LobbyPC->RequestChangeTeam(RequestedTeamTag);
	}
	/*
	
	// 선택한 팀 몇명인지 확인
	int32 CurrentTeamCount = 0;
	for (const FSGPlayerLobbyInfo& Info  : PlayerInfos)
	{
		if (Info.TeamTag == RequestedTeamTag) CurrentTeamCount++;
	}
	// [주의] 
	//   기존의 로컬에서 PlayerInfos를 바꾸고 RefreshLobby()를 돌리던 코드들은 제거하거나 주석 처리합니다.
	//   서버에서 팀 변경 처리가 완료되면, 서버가 클라이언트들의 SetPlayerInfos()를 호출하여 
	//   자연스럽게 UI가 브로드캐스트 리프레시되도록 유도하는 것이 정석입니다.
	// 대기중인 사람 태 그 
	FGameplayTag WaitingTag = FGameplayTag::RequestGameplayTag(FName("Team.Waiting"));
	// 대기중인 사람 몇명인지 확인
	int32 MaxCapacity = (RequestedTeamTag == WaitingTag) ? 6 : 3;
	
	// 만석이면 취소
	if (CurrentTeamCount >= MaxCapacity)
	{
		UE_LOG(LogTemp, Warning, TEXT("해당 팀은 이미 꽉 찼습니다!"));
		return;
	}
	// 맞으면 실행
	PlayerInfos[LocalPlayerIndex].TeamTag = RequestedTeamTag;
	PlayerInfos[LocalPlayerIndex].bIsReady = false;
	
	RefreshLobby();
	UpdateReadyButtonText();
	 */
	
}
