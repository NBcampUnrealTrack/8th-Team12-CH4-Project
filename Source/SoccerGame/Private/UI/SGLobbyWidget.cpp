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
	
	// 오너와 관계없이 현재 내 화면 인스턴스의 진짜 로컬 컨트롤러 강제 참조
	ASGLobbyPlayerController* LobbyPC = nullptr;
	if (GEngine && GetWorld())
	{
		LobbyPC = Cast<ASGLobbyPlayerController>(GEngine->GetFirstLocalPlayerController(GetWorld()));
	}
    
	if (LobbyPC)
	{
		// 내 컨트롤러를 통해 서버에 레디 상태 토글 요청전달
		LobbyPC->SellectReady();
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
}

void USGLobbyWidget::UpdateReadyButtonText()
{
	if (!Text_ReadyButton) return;
	
	APlayerController* LocalPC = GetOwningPlayer();
	if (!LocalPC) return;
	
	ASGLobbyPlayerState* MyPlayerState = Cast<ASGLobbyPlayerState>(LocalPC->PlayerState);
	if (!MyPlayerState) return;
	
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
	if (APlayerController* PC = GEngine ? GEngine->GetFirstLocalPlayerController(GetWorld()) : nullptr)
	{
		if (ULocalPlayer* LP = PC->GetLocalPlayer())
		{
			// 이 위젯이 떠 있는 화면의 진짜 로컬 플레이어 ID (보통 단일 PC 멀티플레이 테스트(PIE)에서는 0, 1, 2, 3 번으로 매핑됨)
			LocalPlayerIndex = LP->GetControllerId();
		}
	}
	
	if (!PlayerInfos.IsValidIndex(LocalPlayerIndex)) return;
	//if (PlayerInfos[LocalPlayerIndex].TeamTag == RequestedTeamTag) return;
	
	// 위젯을 소유한 플레이어 컨트롤러 가져오기
	ASGLobbyPlayerController* LobbyPC = Cast<ASGLobbyPlayerController>(GetOwningPlayer());
	if (LobbyPC)
	{
		// 내 컨트롤러를 통해 서버에 팀 변경 요청 전달
		LobbyPC->RequestChangeTeam(RequestedTeamTag);
	}
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
	
	CurrentIndex = (CurrentIndex - 1) % CharacterList.Num();
	UpdateThumbnail();
}
	



