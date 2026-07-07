// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/SGLobbyWidget.h"
#include "UI/SGPlayerSlotWidget.h"
#include "Components/Button.h"
#include "Components/TextBlock.h"
#include "Components/VerticalBox.h"
#include "GameState/SGLobbyGameState.h"
#include "Kismet/GameplayStatics.h"

#include "SoccerGame/Public/PlayerController/SGLobbyPlayerController.h"

constexpr int32 MaxPlayers = 6;
constexpr int32 MaxBLueTeam = 3;
constexpr int32 MaxRedTeam = 3;

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
	
}

void USGLobbyWidget::NativeTick(const FGeometry& MyGeometry, float InDeltaTime)
{
	Super::NativeTick(MyGeometry, InDeltaTime);
	
	// GameState에서 받아온 타이머 SetText
	if (ASGLobbyGameState* GS = Cast<ASGLobbyGameState>(UGameplayStatics::GetGameState(this)))
	{
		if (Text_StartTimer)
		{
			if (GS->ReplicatedCountdownTime >= 0)
			{
				Text_StartTimer->SetVisibility(ESlateVisibility::Visible);
				Text_StartTimer->SetText(FText::AsNumber(GS->ReplicatedCountdownTime));
			}
			else
			{
				Text_StartTimer->SetVisibility(ESlateVisibility::Hidden);
			}
		}
	}
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
			if (CurrentBlueIndex < MaxBLueTeam && BlueTeamSlots[CurrentBlueIndex])
			{
				BlueTeamSlots[CurrentBlueIndex]->SetPlayerSlotInfo(PlayerInfo.UserName, PlayerInfo.bIsReady, PlayerInfo.TeamTag);
				CurrentBlueIndex++;
			}
		}
		else if (PlayerInfo.TeamTag == RedTag)
		{
			if (CurrentRedIndex < MaxRedTeam && RedTeamSlots[CurrentRedIndex])
			{
				RedTeamSlots[CurrentRedIndex]->SetPlayerSlotInfo(PlayerInfo.UserName, PlayerInfo.bIsReady, PlayerInfo.TeamTag);
				CurrentRedIndex++;
			}
		}
		else if (PlayerInfo.TeamTag == WaitingTag)
		{
			if (CurrentWaitingIndex < MaxPlayers && WaitingSlots[CurrentWaitingIndex])
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
	
	//  이 위젯을 소유한 로컬 PlayerController 가져오기
	ASGLobbyPlayerController* LobbyPC = Cast<ASGLobbyPlayerController>(GetOwningPlayer());
	if (LobbyPC)
	{
		//  내 컨트롤러를 통해 서버에 레디 상태 토글 요청전달
		LobbyPC->SellectReady();
	}
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

void USGLobbyWidget::UpdateCountdownText(int32 NewTime)
{
	// [확인] meta = (BindWidget) 덕분에 에디터의 Text_StartTimer 가 이 포인터에 자동 연동되어 있습니다.
	if (!Text_StartTimer) return;

	// 1. 카운트다운이 취소되었거나 끝난 경우 (-1 이하 혹은 0초 도달 시)
	if (NewTime <= 0 || NewTime == -1)
	{
		// UI 화면에서 완전히 숨김 처리합니다.
		Text_StartTimer->SetVisibility(ESlateVisibility::Collapsed);
	}
	else
	{
		// 숨겨져 있었다면 다시 화면에 보이도록 설정합니다.
		Text_StartTimer->SetVisibility(ESlateVisibility::Visible);

		// 출력하고 싶은 텍스트 포맷 생성
		FString CountdownString = FString::Printf(TEXT("게임 시작까지 %d..."), NewTime);
        
		// UI 텍스트 업데이트
		Text_StartTimer->SetText(FText::FromString(CountdownString));
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
}
	



