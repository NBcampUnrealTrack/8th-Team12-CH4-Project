// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/SGLobbyWidget.h"
#include "UI/SGPlayerSlotWidget.h"
#include "Components/Button.h"
#include "Components/TextBlock.h"
#include "Components/Image.h"
#include "Engine/Texture2D.h"
#include "SoccerGame/Public/PlayerController/SGLobbyPlayerController.h"
#include "Character/SGCharacterDataAsset.h"


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
			BlueSlot->OnSlotClicked.AddUniqueDynamic(this, &USGLobbyWidget::HandleSlotClicked);
		}
	}
	
	for (auto* RedSlot : RedTeamSlots)
	{
		if (RedSlot)
		{
			RedSlot->SetSlotTeamTag(RedTag);
			RedSlot->OnSlotClicked.AddUniqueDynamic(this, &USGLobbyWidget::HandleSlotClicked);
		}
	}
	
	for (auto* WaitingSlot : WaitingSlots)
	{
		if (WaitingSlot)
		{
			WaitingSlot->SetSlotTeamTag(WaitingTag);
			WaitingSlot->OnSlotClicked.AddUniqueDynamic(this, &USGLobbyWidget::HandleSlotClicked);
		}
	}
	
	if (ReadyButton)
	{
		ReadyButton->OnClicked.AddUniqueDynamic(this, &USGLobbyWidget::OnReadyButtonClicked);
	}
	
	if (IsValid(Button_ChangeUserName))
	{
		Button_ChangeUserName->OnClicked.AddUniqueDynamic(this,&USGLobbyWidget::OnClickedChangeUsernameButton);
	}
	
	if (IsValid(Text_StartTimer))
	{
		Text_StartTimer->SetVisibility(ESlateVisibility::Collapsed);
	}
	
	if (IsValid(Button_BackToMenu))
	{
		Button_BackToMenu->OnClicked.AddUniqueDynamic(this,&USGLobbyWidget::OnClickedBackMainMenuButton);
	}
	
	if (IsValid(Button_Left))
	{
		Button_Left->OnClicked.AddUniqueDynamic(this, &USGLobbyWidget::OnPrevButtonClicked);
	}
	
	if (IsValid(Button_Right))
	{
		Button_Right->OnClicked.AddUniqueDynamic(this, &USGLobbyWidget::OnNextButtonClicked);
	}
	
	UpdateReadyButtonText();
	BindLobbyPlayerState();
	
}

void USGLobbyWidget::NativeDestruct()
{
	
	// 델리게이트 바인딩 해제
	APlayerController* LobbyPC = GetOwningPlayer();
	if (IsValid(LobbyPC))
	{
		ASGLobbyPlayerState* LobbyPS = Cast<ASGLobbyPlayerState>(LobbyPC->PlayerState);
		if (IsValid(LobbyPS))
		{
			LobbyPS->OnTeamChanged.RemoveDynamic(this, &USGLobbyWidget::RefreshCharacterSelection);
		}
	}
	
	// 버튼 바인딩 해제
	if (IsValid(Button_BackToMenu))
	{
		Button_BackToMenu->OnClicked.RemoveDynamic(this,&USGLobbyWidget::OnClickedBackMainMenuButton);
	}
	if (IsValid(ReadyButton))
	{
		ReadyButton->OnClicked.RemoveDynamic(this,&USGLobbyWidget::OnReadyButtonClicked);
	}
	if (IsValid(Button_ChangeUserName))
	{
		Button_ChangeUserName->OnClicked.RemoveDynamic(this,&USGLobbyWidget::OnClickedChangeUsernameButton);
	}
	
	for (USGPlayerSlotWidget* BlueSlot :BlueTeamSlots)
	{
		if (IsValid(BlueSlot))
		{
			BlueSlot->OnSlotClicked.RemoveDynamic(this,&USGLobbyWidget::HandleSlotClicked);
		}
	}

	for (USGPlayerSlotWidget* RedSlot :RedTeamSlots)
	{
		if (IsValid(RedSlot))
		{
			RedSlot->OnSlotClicked.RemoveDynamic(this,&USGLobbyWidget::HandleSlotClicked);
		}
	}

	for (USGPlayerSlotWidget* WaitingSlot :WaitingSlots)
	{
		if (IsValid(WaitingSlot))
		{
			WaitingSlot->OnSlotClicked.RemoveDynamic(this,&USGLobbyWidget::HandleSlotClicked);
		}
	}
	
	if (IsValid(Button_BackToMenu))
	{
		Button_BackToMenu->OnClicked.RemoveDynamic(this, &USGLobbyWidget::USGLobbyWidget::OnClickedBackMainMenuButton);	
	}
	
	Super::NativeDestruct();
	
}

void USGLobbyWidget::SetPlayerInfos(const TArray<FSGPlayerLobbyInfo>& InPlayerInfos)
{
	
	PlayerInfos = InPlayerInfos;
	
	RefreshLobby();
	UpdateReadyButtonText();
	
}

void USGLobbyWidget::RefreshLobby()
{
	
	// 플레이어 슬롯 리셋
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
		
	// 현재 비어있는 첫 슬롯 인덱스
	int CurrentBlueIndex = 0;
	int CurrentRedIndex = 0;
	int CurrentWaitingIndex = 0;
	
	// 게임 플레이 태그 가져오기
	FGameplayTag BlueTag = FGameplayTag::RequestGameplayTag(FName("Team.Blue"));
	FGameplayTag RedTag = FGameplayTag::RequestGameplayTag(FName("Team.Red"));
	FGameplayTag WaitingTag = FGameplayTag::RequestGameplayTag(FName("Team.Waiting"));
	
	// 플레이어 슬롯에 플레이어 정보 등록
	for (const FSGPlayerLobbyInfo& PlayerInfo : PlayerInfos)
	{
		if (PlayerInfo.TeamTag == BlueTag)
		{	
			// 현재 인덱스 < 팀 최대 인원수이고 해당 인덱스의 슬롯이 유효할 때
			if (CurrentBlueIndex < MaxBLueTeam && BlueTeamSlots[CurrentBlueIndex])
			{
				// 플레이어 슬롯에 로컬 플레이어 정보 등록
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

void USGLobbyWidget::BindLobbyPlayerState()
{
	
	APlayerController* LobbyPC = GetOwningPlayer();
	if (!IsValid(LobbyPC)) return;
	
	ASGLobbyPlayerState* LobbyPS = Cast<ASGLobbyPlayerState>(LobbyPC->PlayerState);
	if (!IsValid(LobbyPS)) return;
	
	LobbyPS->OnTeamChanged.AddUniqueDynamic(this, &USGLobbyWidget::RefreshCharacterSelection);
	
	RefreshCharacterSelection();
	
}

void USGLobbyWidget::OnReadyButtonClicked()
{
	
	ASGLobbyPlayerController* LobbyPC = GetOwningPlayer<ASGLobbyPlayerController>();
	if (!IsValid(LobbyPC)) return;

	ASGLobbyPlayerState* LobbyPS = LobbyPC->GetPlayerState<ASGLobbyPlayerState>();
	if (!IsValid(LobbyPS)) return;

	// Ready 상태로 전환할 때만 선택한 캐릭터를 서버에 전달합니다.
	const bool bWillBecomeReady = !LobbyPS->IsReady();
	
	if (bWillBecomeReady)
	{
		const TArray<USGCharacterDataAsset*> FilteredList = GetFilteredCharacterList();
		if (!FilteredList.IsValidIndex(CurrentIndex) || !IsValid(FilteredList[CurrentIndex])) return;

		const FGameplayTag SelectedCharacterTag = FilteredList[CurrentIndex]->CharacterTag;
		if (!SelectedCharacterTag.IsValid()) return;
		
		LobbyPC->RequestChangeCharacter(SelectedCharacterTag);
	}
	
	LobbyPC->SellectReady();
	
}

void USGLobbyWidget::UpdateReadyButtonText()
{
	
	if (!Text_ReadyButton) return;
	
	APlayerController* LocalPC = GetOwningPlayer();
	if (!LocalPC) return;
	
	ASGLobbyPlayerState* MyPlayerState = Cast<ASGLobbyPlayerState>(LocalPC->PlayerState);
	if (!MyPlayerState) return;
	
	// 버튼 기존 스타일 껍데기 가져오기
	FButtonStyle NewStyle = ReadyButton->GetStyle();
	UTexture2D* Normal;
	UTexture2D* Hover;
	UTexture2D* Pressed;
	
	if (MyPlayerState->bIsReady == false)
	{
		Text_ReadyButton->SetText(FText::FromString("Ready"));	
		Normal = Image_Ready_Normal;
		Hover = Image_Ready_Hover;
		Pressed = Image_Ready_Pressed;
		
		// 캐릭터 변경 버튼 잠금해제
		if (Button_Left) Button_Left->SetIsEnabled(true);
		if (Button_Right) Button_Right->SetIsEnabled(true);
		if (Button_ChangeUserName) Button_ChangeUserName->SetIsEnabled(true);
		if (Button_BackToMenu) Button_BackToMenu->SetIsEnabled(true);
	}
	else
	{
		Text_ReadyButton->SetText(FText::FromString("Cancel"));
		
		Normal = Image_Cancel_Normal;
		Hover = Image_Cancel_Hover;
		Pressed = Image_Cancel_Pressed;
		
		// 캐릭터 변경 버튼 잠금
		if (Button_Left) Button_Left->SetIsEnabled(false);
		if (Button_Right) Button_Right->SetIsEnabled(false);
		if (Button_ChangeUserName) Button_ChangeUserName->SetIsEnabled(false);
		if (Button_BackToMenu) Button_BackToMenu->SetIsEnabled(false);
	}
	
	if (Normal) NewStyle.Normal.SetResourceObject(Normal);
	if (Hover) NewStyle.Hovered.SetResourceObject(Hover);
	if (Pressed) NewStyle.Pressed.SetResourceObject(Pressed);
	
	ReadyButton->SetStyle(NewStyle);
	
}

void USGLobbyWidget::UpdateCountdownText(int32 NewTime)
{
	
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
	
	if (!RequestedTeamTag.IsValid()) return;
	ASGLobbyPlayerController* LobbyPlayerController = GetOwningPlayer<ASGLobbyPlayerController>();
	if (!IsValid(LobbyPlayerController)) return;
	
	// 플레이어 컨트롤러에게 팀 변경 요청
	LobbyPlayerController->RequestChangeTeam(RequestedTeamTag);
	
}

TArray<USGCharacterDataAsset*> USGLobbyWidget::GetFilteredCharacterList()
{
	
	TArray<USGCharacterDataAsset*> FilteredList;
	
	APlayerController* LobbyPC = GetOwningPlayer();
	ASGLobbyPlayerState* LobbyPS = LobbyPC ? Cast<ASGLobbyPlayerState>(LobbyPC->PlayerState) : nullptr;
	if (!LobbyPS) return FilteredList;
	
	FString MyTeamTagString = LobbyPS->GetTeamTag().GetTagName().ToString();
	
	// 팀 태그에 따라 캐릭터 필터링
	for (auto* Character : CharacterList)
	{
		if (Character)
		{
			FString CharTagString = Character->CharacterTag.ToString();
			
			if (CharTagString.Contains(MyTeamTagString))
			{
				FilteredList.Add(Character);
			}
		}
	}
	
	return FilteredList;
}

void USGLobbyWidget::OnNextButtonClicked()
{
	TArray<USGCharacterDataAsset*> FilteredList = GetFilteredCharacterList();
	if (FilteredList.Num() == 0) return;
	
	// 인덱스 증가 및 순환
	CurrentIndex = (CurrentIndex + 1) % FilteredList.Num();
	
	// 썸네일 업데이트
	if (Image_Character && FilteredList.IsValidIndex(CurrentIndex))
	{
		Image_Character->SetBrushFromTexture(FilteredList[CurrentIndex]->Thumbnail);
	}
}

void USGLobbyWidget::OnPrevButtonClicked()
{
	TArray<USGCharacterDataAsset*> FilteredList = GetFilteredCharacterList();
	if (FilteredList.Num() == 0) return;
	
	// 플레이어 팀 태그에 따라 필터링된 캐릭터 리스트 인덱스 -1 (0 이하가 되면 순환)
	CurrentIndex = (CurrentIndex - 1 + FilteredList.Num()) % FilteredList.Num();
	
	// 해당 인덱스의 캐릭터 이미지 썸네일로 설정
	if (Image_Character && FilteredList.IsValidIndex(CurrentIndex))
	{
		Image_Character->SetBrushFromTexture(FilteredList[CurrentIndex]->Thumbnail);
	}	
}

void USGLobbyWidget::RefreshCharacterSelection()
{
	APlayerController* LobbyPC = GetOwningPlayer();
	ASGLobbyPlayerState* LobbyPS = LobbyPC ? Cast<ASGLobbyPlayerState>(LobbyPC->PlayerState) : nullptr;
	if (!LobbyPS) return;
	
	bool bIsWaiting = LobbyPS->GetTeamTag().GetTagName().ToString().Contains(TEXT("Waiting"));
	if (Image_Character)
	{
		// Waiting 상태가 아닐 때만 캐릭터 이미지 보여주기.
		Image_Character->SetVisibility(bIsWaiting ? ESlateVisibility::Hidden : ESlateVisibility::Visible);
	}
	
	if (Button_Left && Button_Right)
	{
		// Waiting 상태가 아닐 때만 캐릭터 변경 버튼 보여주기.
		Button_Left->SetVisibility(bIsWaiting ? ESlateVisibility::Hidden : ESlateVisibility::Visible);
		Button_Right->SetVisibility(bIsWaiting ? ESlateVisibility::Hidden : ESlateVisibility::Visible);
	}
	
	// [방어코드] CurrentIndex가 FilteredList.Num()을 넘어설 경우 인덱스 0으로 설정해서 캐릭터 이미지 변경
	if (!bIsWaiting)
	{
		TArray<USGCharacterDataAsset*> FilteredList = GetFilteredCharacterList();
	
		if (FilteredList.Num() > 0)
		{
			// 현재 인덱스가 필터링된 리스트 요소 개수를 넘어서면 0으로 설정
			if (CurrentIndex >= FilteredList.Num())
			{
				CurrentIndex = 0;
			}
			
			// 캐릭터 이미지 썸네일로 설정
			if (Image_Character && FilteredList.IsValidIndex(CurrentIndex))
			{
				Image_Character->SetBrushFromTexture(FilteredList[CurrentIndex]->Thumbnail);
			}
		}
	}
	
}

void USGLobbyWidget::OnClickedBackMainMenuButton()
{
	ASGLobbyPlayerController* LobbyPC =GetOwningPlayer<ASGLobbyPlayerController>();
	if (!IsValid(LobbyPC)) return;
	
	LobbyPC->ClientToMainMenu();
}

void USGLobbyWidget::OnClickedChangeUsernameButton()
{
	ASGLobbyPlayerController* LobbyPC =
		Cast<ASGLobbyPlayerController>(GetOwningPlayer());

	if (!LobbyPC) return;

	LobbyPC->OpenChangeUsernameWidget();
}
