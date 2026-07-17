// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/SGLobbyWidget.h"
#include "UI/SGPlayerSlotWidget.h"
#include "Components/Button.h"
#include "Components/TextBlock.h"
#include "Components/VerticalBox.h"
#include "Components/Image.h"
#include "GameState/SGLobbyGameState.h"
#include "Kismet/GameplayStatics.h"
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
			// [안전장치] 중복 바인딩을 막기 위해 기존에 연결되어 있던 것을 먼저 제거 후 다시 연결합니다.
			BlueSlot->OnSlotClicked.RemoveDynamic(this, &USGLobbyWidget::HandleSlotClicked);
			BlueSlot->OnSlotClicked.AddDynamic(this, &USGLobbyWidget::HandleSlotClicked);
		}
	}
	for (auto* RedSlot : RedTeamSlots)
	{
		if (RedSlot)
		{
			RedSlot->SetSlotTeamTag(RedTag);
			RedSlot->OnSlotClicked.RemoveDynamic(this, &USGLobbyWidget::HandleSlotClicked);
			RedSlot->OnSlotClicked.AddDynamic(this, &USGLobbyWidget::HandleSlotClicked);
		}
	}
	for (auto* WaitingSlot : WaitingSlots)
	{
		if (WaitingSlot)
		{
			WaitingSlot->SetSlotTeamTag(WaitingTag);
			WaitingSlot->OnSlotClicked.RemoveDynamic(this, &USGLobbyWidget::HandleSlotClicked);
			WaitingSlot->OnSlotClicked.AddDynamic(this, &USGLobbyWidget::HandleSlotClicked);
		}
	}
	
	if (ReadyButton)
	{
		ReadyButton->OnClicked.RemoveDynamic(this,&USGLobbyWidget::OnReadyButtonClicked);
		ReadyButton->OnClicked.AddDynamic(this, &USGLobbyWidget::OnReadyButtonClicked);
	}
	if (IsValid(Button_ChangeUserName))
	{
		Button_ChangeUserName->OnClicked.RemoveDynamic(this,&USGLobbyWidget::OnClickedChangeUsernameButton);

		Button_ChangeUserName->OnClicked.AddDynamic(this,&USGLobbyWidget::OnClickedChangeUsernameButton);
	}
	
	if (IsValid(Text_StartTimer))
	{
		Text_StartTimer->SetVisibility(ESlateVisibility::Collapsed);
	}
	
	if (IsValid(Button_ChangeUserName))
	{
		UE_LOG(
			LogTemp,
			Warning,
			TEXT("[LobbyWidget] Button_ChangeUserName 바인딩 성공")
		);

		Button_ChangeUserName->OnClicked.RemoveDynamic(
			this,
			&USGLobbyWidget::OnClickedChangeUsernameButton
		);

		Button_ChangeUserName->OnClicked.AddDynamic(
			this,
			&USGLobbyWidget::OnClickedChangeUsernameButton
		);
	}
	else
	{
		UE_LOG(
			LogTemp,
			Error,
			TEXT("[LobbyWidget] Button_ChangeUserName 바인딩 실패")
		);
	}
}

void USGLobbyWidget::NativeDestruct()
{

	UE_LOG(
		LogTemp,
		Error,
		TEXT(
			"[LobbyWidget Construct] "
			"Widget=%s, Class=%s, OwningPlayer=%s, Map=%s"
		),
		*GetNameSafe(this),
		*GetNameSafe(GetClass()),
		*GetNameSafe(GetOwningPlayer()),
		GetWorld()
			? *GetWorld()->GetMapName()
			: TEXT("None")
	);
	if (IsValid(ReadyButton))
	{
		ReadyButton->OnClicked.RemoveDynamic(
			this,
			&USGLobbyWidget::OnReadyButtonClicked
		);
	}
	if (IsValid(Button_ChangeUserName))
	{
		Button_ChangeUserName->OnClicked.RemoveDynamic(
			this,
			&USGLobbyWidget::OnClickedChangeUsernameButton
		);
	}


	for (USGPlayerSlotWidget* BlueSlot :
		 BlueTeamSlots)
	{
		if (IsValid(BlueSlot))
		{
			BlueSlot->OnSlotClicked.RemoveDynamic(this,&USGLobbyWidget::HandleSlotClicked);
		}
	}

	for (USGPlayerSlotWidget* RedSlot :
		 RedTeamSlots)
	{
		if (IsValid(RedSlot))
		{
			RedSlot->OnSlotClicked.RemoveDynamic(this,&USGLobbyWidget::HandleSlotClicked);
		}
	}

	for (USGPlayerSlotWidget* WaitingSlot :
		 WaitingSlots)
	{
		if (IsValid(WaitingSlot))
		{
			WaitingSlot->OnSlotClicked.RemoveDynamic(this,&USGLobbyWidget::HandleSlotClicked);
		}
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
	ASGLobbyPlayerController* LobbyPC =GetOwningPlayer<ASGLobbyPlayerController>();
	// LocalPlayerIndex 유효성 검사
	if (!IsValid(LobbyPC))
	{
		return;
	}
	
	// 선택한 캐릭터 전달
	if (CharacterList.IsValidIndex(CurrentIndex))
	{
		FGameplayTag SelectedCharacterTag = CharacterList[CurrentIndex]->CharacterTag;
		if (LobbyPC)
		{
			// TODO: 선택된 캐릭터 전송하는 함수 호출
		}
	}
	LobbyPC->SellectReady();
}

void USGLobbyWidget::UpdateReadyButtonText()
{
	if (!Text_ReadyButton)
	{
		return;
	}
	
	APlayerController* LocalPC = GetOwningPlayer();
	if (!LocalPC)
	{
		return;
	}
	
	ASGLobbyPlayerState* MyPlayerState = Cast<ASGLobbyPlayerState>(LocalPC->PlayerState);
	if (!MyPlayerState)
	{
		return;
	}
	
	if (MyPlayerState->bIsReady == false)
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
	if (!IsValid(Text_StartTimer))
	{
		return;
	}

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
	if (!RequestedTeamTag.IsValid())
	{
		return;
	}
	ASGLobbyPlayerController* LobbyPlayerController =
		GetOwningPlayer<ASGLobbyPlayerController>();

	if (!IsValid(LobbyPlayerController))
	{
		return;
	}
	LobbyPlayerController->RequestChangeTeam(RequestedTeamTag);
}

void USGLobbyWidget::UpdateThumbnail()
{
	if (!Image_Character || CharacterList.Num() == 0)
	{
		if (GEngine)
		{
			GEngine->AddOnScreenDebugMessage(-1, 10.f, FColor::Red, TEXT("[SGLobbyWidget] 캐릭터 이미지 슬롯이 존재하지 않거나, 캐릭터 에셋 데이터가 존재하지 않습니다!"));
		}
	}
	
	USGCharacterDataAsset* SelectedCharacter = CharacterList[CurrentIndex];
	
	if (SelectedCharacter && SelectedCharacter->Thumbnail)
	{
		Image_Character->SetBrushFromTexture(SelectedCharacter->Thumbnail);
	}
}

void USGLobbyWidget::OnNextButtonClicked()
{
	if (CharacterList.Num() == 0) return;
	
	// 인덱스 증가 및 순환
	CurrentIndex = (CurrentIndex + 1) % CharacterList.Num();
	UpdateThumbnail();
	
}

void USGLobbyWidget::OnPrevButtonClicked()
{
	if (CharacterList.Num() == 0) return;
	
	CurrentIndex = (CurrentIndex - 1 + CharacterList.Num()) % CharacterList.Num();
	UpdateThumbnail();
}


void USGLobbyWidget::OnClickedChangeUsernameButton()
{
	ASGLobbyPlayerController* LobbyPC =
		Cast<ASGLobbyPlayerController>(GetOwningPlayer());

	if (!LobbyPC)
	{
		UE_LOG(
			LogTemp,
			Error,
			TEXT(
				"[LobbyWidget] LobbyPlayerController가 없습니다. "
				"OwningPlayer=%s Class=%s"
			),
			*GetNameSafe(GetOwningPlayer()),
			GetOwningPlayer()
				? *GetOwningPlayer()->GetClass()->GetName()
				: TEXT("None")
		);

		return;
	}

	UE_LOG(
		LogTemp,
		Log,
		TEXT("[LobbyWidget] 이름 변경 버튼 클릭")
	);

	LobbyPC->OpenChangeUsernameWidget();
}