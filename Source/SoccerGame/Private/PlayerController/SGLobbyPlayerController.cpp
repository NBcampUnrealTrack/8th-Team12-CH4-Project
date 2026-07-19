// Fill out your copyright notice in the Description page of Project Settings.


#include "PlayerController/SGLobbyPlayerController.h"
#include "GameFramework/PlayerState.h"
#include "Blueprint/UserWidget.h"
#include "SoccerGame/Public/UI/SGLobbyWidget.h"
#include "SoccerGame/Public/UI/SGChangeUsernameWidget.h"
#include "GameMode/SGLobbyGameMode.h"
#include "PlayerState/SGLobbyPlayerState.h"
#include "Multiplay/SGMultiplayGameInstance.h"
#include "SoccerGame/Public/Instance/SGPlayerGameInstanceSubsystem.h"

void ASGLobbyPlayerController::BeginPlay()
{
	Super::BeginPlay();

	if (!IsLocalController())
	{
		return; 
	}
	if (!IsValid(LobbyWidgetClass))
	{
		return;
	}
	LobbyWidgetInstance = CreateWidget<UUserWidget>(this, LobbyWidgetClass);
	
	if (!IsValid(LobbyWidgetInstance))
	{
		return;
	}

	LobbyWidgetInstance->AddToViewport();
	FInputModeUIOnly Mode;
	SetInputMode(Mode);
	bShowMouseCursor = true;
	
	InitializeLocalPlayerLobbyUI();
}

void ASGLobbyPlayerController::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	if (IsLocalController())
	{
		if (LobbyWidgetInstance && IsLocalController())
		{
			LobbyWidgetInstance->RemoveFromParent();
			LobbyWidgetInstance = nullptr;
		}
		if (IsValid(ChangeUsernameWidgetInstance))
		{
			ChangeUsernameWidgetInstance->RemoveFromParent();
			ChangeUsernameWidgetInstance = nullptr;
		}
	}
	Super::EndPlay(EndPlayReason);
}

void ASGLobbyPlayerController::SellectReady()
{
	ASGLobbyPlayerState* LobbyPlayerState  = GetPlayerState<ASGLobbyPlayerState>();
	if (!IsValid(LobbyPlayerState))
	{
		return;
	}
	const bool bTargetReady = !LobbyPlayerState->IsReady();

	Server_SetReady(bTargetReady);
}

void ASGLobbyPlayerController::RequestChangeTeam_Implementation(FGameplayTag NewTeam)
{
	if (ASGLobbyGameMode* LobbyGM = Cast<ASGLobbyGameMode>(GetWorld()->GetAuthGameMode()))
	{
		LobbyGM->ProcessChangeTeamRequest(this, NewTeam);
	}

}
bool ASGLobbyPlayerController::RequestChangeTeam_Validate(FGameplayTag NewTeam)
{
	return NewTeam.IsValid();
}

void ASGLobbyPlayerController::RequestChangeCharacter_Implementation(FGameplayTag NewCharacterTag)
{
	if (ASGLobbyGameMode* LobbyGM = Cast<ASGLobbyGameMode>(GetWorld()->GetAuthGameMode()))
	{
		LobbyGM->ProcessChangeCharacterRequest(this, NewCharacterTag);
	}
}

bool ASGLobbyPlayerController::RequestChangeCharacter_Validate(FGameplayTag NewCharacterTag)
{
	return NewCharacterTag.IsValid();
}
void ASGLobbyPlayerController::Client_UpdateLobbyUI_Implementation(const TArray<FSGPlayerLobbyInfo>& InPlayerInfos)
{
	if (!IsLocalController())
	{
		return;
	}
	if (!IsValid(LobbyWidgetInstance))
	{
		return;
	}
	USGLobbyWidget* LobbyWidget = Cast<USGLobbyWidget>(LobbyWidgetInstance);
	if (!IsValid(LobbyWidget))
	{
		return ;
	}
	LobbyWidget->SetPlayerInfos(InPlayerInfos);
}

void ASGLobbyPlayerController::TimeUIUpdate(int32 NewTime)
{
	if (!IsLocalController())
	{
		return;
	}

	if (!IsValid(LobbyWidgetInstance))
	{
		return;
	}
	// 본인 클래스의 멤버 변수이므로 안전하게 접근 가능!
	if (USGLobbyWidget* LobbyWidget = Cast<USGLobbyWidget>(LobbyWidgetInstance))
	{
		// 선택지 A 적용: 위젯의 카운트다운 전용 함수를 안전하게 호출
		LobbyWidget->UpdateCountdownText(NewTime);
            
		UE_LOG(LogTemp, Log, TEXT("[LobbyPC] 위젯 카운트다운 텍스트 업데이트 성공: %d초"), NewTime);
	}

}

void ASGLobbyPlayerController::InitializeLocalPlayerLobbyUI()
{
	if (!IsLocalController())
	{
		return;
	}
	if (!IsValid(LobbyWidgetInstance))
	{
		return;
	}
	USGLobbyWidget* LobbyWidget = Cast<USGLobbyWidget>(LobbyWidgetInstance);
	if (!IsValid(LobbyWidget))
	{
		return;
	}
	FSGPlayerLobbyInfo MyInfo;
	ASGLobbyPlayerState* LobbyPlayerState =GetPlayerState<ASGLobbyPlayerState>();
	if (!IsValid(LobbyPlayerState))
	{
		UE_LOG(LogTemp,Warning,TEXT("[LobbyPC] PlayerState가 아직 준비되지 않았습니다."));
		return;
	}
	LobbyWidget->UpdateReadyButtonText();

	
	
	//MyInfo.UserName =LobbyPlayerState->CustomPlayerName.IsEmpty()
	//		? LobbyPlayerState->GetPlayerName()
	//		: LobbyPlayerState->CustomPlayerName;
	//
	//MyInfo.TeamTag =LobbyPlayerState->GetTeamTag();
	//
	//MyInfo.bIsReady =LobbyPlayerState->IsReady();
	//
	//LobbyWidget->AddPlayerInfos(MyInfo);
	//LobbyWidget->RefreshLobby();
	//LobbyWidget->UpdateReadyButtonText();
	/*
	//현재 화면에 생성되어 있는 로비 위젯 인스턴스가 있는지 확인
	ASGLobbyPlayerState* MyLobbyPS = GetPlayerState<ASGLobbyPlayerState>();
	MyLobbyPS->CurrentTeamTag = FGameplayTag::RequestGameplayTag(FName("Team.Waiting"));
	// 위젯의 PlayerInfos에 집어넣을 내 정보 구조체 생성
	FSGPlayerLobbyInfo MyInfo;
	FString FinalClientName = TEXT("UnknownClient");

	int32 RealClientID = GPlayInEditorID;

	// 2. 서버인지 클라이언트인지에 따라 이름 포맷 결정
	if (GetWorld() && GetWorld()->GetNetMode() == NM_Client)
	{
		// 클라이언트 창들은 "Client 1", "Client 2" 등으로 이름 부여
		FinalClientName = FString::Printf(TEXT("Client %d"), RealClientID);
	}
	else
	{
		// 리슨 서버(방장)의 경우 "Host (Server)" 또는 "Client 0" 으로 표현
		FinalClientName = TEXT("Host (Server)");
	}

	// PlayerState에도 변경된 이름을 반영해 둡니다 (서버 동기화용)
	MyLobbyPS->CustomPlayerName = FinalClientName;
	MyInfo.UserName = FinalClientName;
	MyInfo.TeamTag = MyLobbyPS->CurrentTeamTag;
	MyInfo.bIsReady = MyLobbyPS->bIsReady;

	LobbyWidget->AddPlayerInfos(MyInfo);

	LobbyWidget->RefreshLobby();
	LobbyWidget->UpdateReadyButtonText();
	 */
	
}

void ASGLobbyPlayerController::SaveDataToSubsystem()
{
	const FString PCName = GetNameSafe(this);
	ASGLobbyPlayerState* LobbyPS = GetPlayerState<ASGLobbyPlayerState>();
	if (!LobbyPS)
	{
		return;
	}

	// 2. GameInstanceSubsystem 가져오기
	UGameInstance* GI = GetGameInstance();
	USGPlayerGameInstanceSubsystem* DataSubsystem = GI ? GI->GetSubsystem<USGPlayerGameInstanceSubsystem>() : nullptr;
	if (!DataSubsystem)
	{
		return;
	}

	FUniqueNetIdRepl UniqueId = LobbyPS->GetUniqueId();
	if (!UniqueId.IsValid())
	{
		return;
	}

	FPlayerBackupData DataToSave;
	DataToSave.PlayerName = LobbyPS->CustomPlayerName.IsEmpty() ? 
		LobbyPS->GetPlayerName() : LobbyPS->CustomPlayerName;
	DataToSave.PlayerTeam = LobbyPS->GetTeamTag();
	DataToSave.SelectedCharacterTag = LobbyPS->GetSelectedCharacterTag();
	DataToSave.Score = 0; 

	DataSubsystem->SavePlayerData(UniqueId, DataToSave);
}

void ASGLobbyPlayerController::RequestChangeUsername(const FString& NewUsername)
{
	if (!IsLocalController())
	{
		return;
	}

	Server_ChangeUsername(NewUsername);
}

void ASGLobbyPlayerController::OpenChangeUsernameWidget()
{
	if (!IsLocalController())
	{
		return;
	}

	if (!IsValid(ChangeUsernameWidgetClass))
	{
		UE_LOG(LogTemp,Error,TEXT("[LobbyPC] ChangeUsernameWidgetClass가 설정되지 않았습니다."));
		return;
	}

	if (IsValid(ChangeUsernameWidgetInstance))
	{
		if (!ChangeUsernameWidgetInstance->IsInViewport())
		{
			ChangeUsernameWidgetInstance->AddToViewport(10);
		}

		return;
	}

	ChangeUsernameWidgetInstance =CreateWidget<USGChangeUsernameWidget>(
		this,ChangeUsernameWidgetClass);

	if (!IsValid(ChangeUsernameWidgetInstance))
	{
		UE_LOG(LogTemp,Error,TEXT("[LobbyPC] 이름 변경 위젯 생성 실패"));

		return;
	}

	// 매직넙허 수정 확인
	ChangeUsernameWidgetInstance->AddToViewport(10);
	FInputModeUIOnly InputMode;
	SetInputMode(InputMode);
	bShowMouseCursor = true;
}
void ASGLobbyPlayerController::CloseChangeUsernameWidget()
{
	if (IsValid(ChangeUsernameWidgetInstance))
	{
		ChangeUsernameWidgetInstance->RemoveFromParent();
		ChangeUsernameWidgetInstance = nullptr;
	}

	FInputModeUIOnly InputMode;
	SetInputMode(InputMode);

	bShowMouseCursor = true;

}

void ASGLobbyPlayerController::ClientToMainMenu_Implementation()
{
	if (!IsLocalController())
	{
		return;
	}
	USGMultiplayGameInstance* MultiplayerGameInstance =GetGameInstance<USGMultiplayGameInstance>();
	if (!IsValid(MultiplayerGameInstance))
	{
		return;
	}
	MultiplayerGameInstance->LeaveSessionAndReturnToMainMenu();
}
void ASGLobbyPlayerController::Server_ChangeUsername_Implementation(const FString& NewUsername)
{
	const FString TrimmedUsername =NewUsername.TrimStartAndEnd();

	if (TrimmedUsername.Len() < 2 ||TrimmedUsername.Len() > 16)
	{
		UE_LOG(LogTemp,Warning,TEXT("[LobbyPC][Server] 유효하지 않은 이름 요청: %s"),
			*TrimmedUsername);

		return;
	}

	ASGLobbyPlayerState* LobbyPlayerState =GetPlayerState<ASGLobbyPlayerState>();
	if (!IsValid(LobbyPlayerState))
	{
		return;
	}

	// 실제 PlayerState 이름 변경
	LobbyPlayerState->SetCustomPlayerName(TrimmedUsername);

	UE_LOG(LogTemp,Log,TEXT("[LobbyPC][Server] 이름 변경 완료: %s"),*TrimmedUsername);
}

void ASGLobbyPlayerController::Server_SetReady_Implementation(bool bNewReadyState)
{
	ASGLobbyPlayerState* SG_PlayerState = GetPlayerState<ASGLobbyPlayerState>();
    if (SG_PlayerState)
    {
       // 플레이어 상태 변경 (값이 변경되면 OnRep에 의해 자동으로 전체 UI 브로드캐스트가 일어남)
       SG_PlayerState->SetReadyState(bNewReadyState);
        
       // 재검사 요청
       ASGLobbyGameMode* TitleGM = Cast<ASGLobbyGameMode>(GetWorld()->GetAuthGameMode());
       if (TitleGM)
       {
          // 기존에 사용하시던 GameMode 측 레디 감지 함수 호출 유지
          TitleGM->OnPlayerReadyChanged();
       }
    }
}

bool ASGLobbyPlayerController::Server_SetReady_Validate(bool bNewReadyState)
{
	return true;
}
