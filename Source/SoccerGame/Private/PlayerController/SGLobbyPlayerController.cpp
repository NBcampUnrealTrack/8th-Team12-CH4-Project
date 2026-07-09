// Fill out your copyright notice in the Description page of Project Settings.


#include "PlayerController/SGLobbyPlayerController.h"
#include "GameFramework/PlayerState.h"
#include "Blueprint/UserWidget.h"
#include "SoccerGame/Public/UI/SGLobbyWidget.h"
#include "GameMode/SGLobbyGameMode.h"
#include "PlayerState/SGLobbyPlayerState.h"
#include "SoccerGame/Public/Instance/SGPlayerGameInstanceSubsystem.h"

void ASGLobbyPlayerController::BeginPlay()
{
	Super::BeginPlay();
	
	if (IsLocalController() == false)
	{
		return; 
	}
	
	if (UIWidgetClass == nullptr)
	{
		UE_LOG(LogTemp, Error, TEXT("UIWidgetClass 없음."));
	}
	
	if (IsValid(UIWidgetClass) == true)
	{
		UIWidgetInstance = CreateWidget<UUserWidget>(this, UIWidgetClass); 
		if (IsValid(UIWidgetInstance) == true)
		{
			UIWidgetInstance->AddToViewport();
			
			FInputModeUIOnly Mode;
			Mode.SetWidgetToFocus(UIWidgetInstance->GetCachedWidget());
			SetInputMode(Mode);
			
			bShowMouseCursor = true;
		}
	}
}

void ASGLobbyPlayerController::SellectReady()
{
	ASGLobbyPlayerState* MyPlayerState = GetPlayerState<ASGLobbyPlayerState>();
	if (MyPlayerState)
	{
		// 내 현재 레디 상태를 반전(토글)시켜서 서버 RPC로 전송
		bool bTargetReady = !MyPlayerState->IsReady();
		Server_SetReady(bTargetReady);
	}
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
	return true;
}
void ASGLobbyPlayerController::Client_UpdateLobbyUI(const TArray<FSGPlayerLobbyInfo>& InPlayerInfos)
{
	// 컨트롤러가 들고 있는 로비 위젯 인스턴스가 안전하게 존재하는지 확인
	if (UIWidgetInstance)
	{
		if (USGLobbyWidget* LobbyWidget = Cast<USGLobbyWidget>(UIWidgetInstance))
		{
			// 위젯에게 방 전체 인원의 최신 종합 데이터를 넘겨줍니다.
			LobbyWidget->SetPlayerInfos(InPlayerInfos);
            
			// 새로 배치
			LobbyWidget->RefreshLobby();
			LobbyWidget->UpdateReadyButtonText();
		}
	}
}

void ASGLobbyPlayerController::TimeUIUpdate(int32 NewTime)
{
	// 본인 클래스의 멤버 변수이므로 안전하게 접근 가능!
	if (UIWidgetInstance)
	{
		if (USGLobbyWidget* LobbyWidget = Cast<USGLobbyWidget>(UIWidgetInstance))
		{
			// 선택지 A 적용: 위젯의 카운트다운 전용 함수를 안전하게 호출
			LobbyWidget->UpdateCountdownText(NewTime);
            
			UE_LOG(LogTemp, Log, TEXT("[LobbyPC] 위젯 카운트다운 텍스트 업데이트 성공: %d초"), NewTime);
		}
	}
}

void ASGLobbyPlayerController::InitializeLocalPlayerLobbyUI()
{
	if (IsValid(UILobbyWidgetClass) == true)
	{
		UIWidgetInstance = CreateWidget<UUserWidget>(this, UILobbyWidgetClass); 
		if (IsValid(UIWidgetInstance) == true)
		{
			UIWidgetInstance->AddToViewport();

			FInputModeUIOnly Mode;
			Mode.SetWidgetToFocus(UIWidgetInstance->GetCachedWidget());
			SetInputMode(Mode);

			bShowMouseCursor = true;
		}
	}
	
	//현재 화면에 생성되어 있는 로비 위젯 인스턴스가 있는지 확인
	if (UIWidgetInstance)
	{
		if (USGLobbyWidget* LobbyWidget = Cast<USGLobbyWidget>(UIWidgetInstance))
		{
			// 2. 내 컴퓨터의 PlayerState(서버 초기화 데이터)를 긁어옵니다.
			if (ASGLobbyPlayerState* MyLobbyPS = GetPlayerState<ASGLobbyPlayerState>())
			{
				MyLobbyPS->CurrentTeamTag = FGameplayTag::RequestGameplayTag(FName("Team.Waiting"));
				// 위젯의 PlayerInfos에 집어넣을 내 정보 구조체 생성
				FSGPlayerLobbyInfo MyInfo;
				// --------------------------------------------------------------------------------
				// ⭐ [수정] 각 클라이언트 창의 고유 번호를 파악해 "Client X" 형태로 이름을 할당합니다.
				// --------------------------------------------------------------------------------
				FString FinalClientName = TEXT("UnknownClient");

				// 1. 현재 에디터 테스트(PIE) 환경에서 진짜 몇 번째 창인지 번호 가져오기
				int32 RealClientID = 0;
#if WITH_EDITOR
				RealClientID = GPlayInEditorID; // 0, 1, 2, 3...
#endif

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
          
				// 위젯에 전달할 구조체에 최종 Client Name 대입
				MyInfo.UserName = FinalClientName;
				// --------------------------------------------------------------------------------
               
				MyInfo.TeamTag = MyLobbyPS->CurrentTeamTag;
				MyInfo.bIsReady = MyLobbyPS->bIsReady;

				LobbyWidget->AddPlayerInfos(MyInfo);

				LobbyWidget->RefreshLobby();
				LobbyWidget->UpdateReadyButtonText();
			}
		}
	}
}

void ASGLobbyPlayerController::SaveDataToSubsystem()
{
	FString PCName = GetNameSafe(this);
	UE_LOG(LogTemp, Log, TEXT("[LobbyPC - SaveStart] %s 가 데이터 백업을 시작합니다."), *PCName);

	// 1. 내 PlayerState 가져오기 및 검증 로그
	ASGLobbyPlayerState* LobbyPS = GetPlayerState<ASGLobbyPlayerState>();
	if (!LobbyPS)
	{
		UE_LOG(LogTemp, Error, TEXT("[LobbyPC - SaveFailed] %s 의 PlayerState를 찾을 수 없습니다!"), *PCName);
		return;
	}

	// 2. GameInstanceSubsystem 가져오기
	UGameInstance* GI = GetGameInstance();
	USGPlayerGameInstanceSubsystem* DataSubsystem = GI ? GI->GetSubsystem<USGPlayerGameInstanceSubsystem>() : nullptr;
	if (!DataSubsystem)
	{
		UE_LOG(LogTemp, Error, TEXT("[LobbyPC - SaveFailed] %s 가 GameInstanceSubsystem에 접근할 수 없습니다!"), *PCName);
		return;
	}

	// 3. UniqueId 유효성 확인 및 로그
	FUniqueNetIdRepl UniqueId = LobbyPS->GetUniqueId();
	if (!UniqueId.IsValid())
	{
		UE_LOG(LogTemp, Error, TEXT("[LobbyPC - SaveFailed] %s 의 UniqueId가 유효하지 않습니다!"), *PCName);
		return;
	}

	// 4. 구조체에 데이터 채우기
	FPlayerBackupData DataToSave;
	DataToSave.PlayerName = LobbyPS->CustomPlayerName.IsEmpty() ? LobbyPS->GetPlayerName() : LobbyPS->CustomPlayerName;
	DataToSave.PlayerTeam = LobbyPS->GetTeamTag();
	DataToSave.Score = 0; 

	// 데이터가 어떻게 가공되었는지 최종 확인 로그
	UE_LOG(LogTemp, Warning, TEXT("[LobbyPC - PackData] %s -> 백업 준비 완료 [Name: %s | Team: %s]"), 
		*PCName, *DataToSave.PlayerName, *DataToSave.PlayerTeam.ToString());

	// 5. 서브시스템에 세이브 (서브시스템 내부 로그가 이어서 출력됩니다)
	DataSubsystem->SavePlayerData(UniqueId, DataToSave);
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
