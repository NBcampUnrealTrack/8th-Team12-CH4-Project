// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/SGGameResultWidget.h"
#include "Components/Button.h"
#include "Components/TextBlock.h"
#include "GameState/SGMainGameState.h"
#include "Kismet/GameplayStatics.h"
#include "PlayerController/SGMainPlayerController.h"


void USGGameResultWidget::NativeConstruct()
{
	Super::NativeConstruct();
	
	// 버튼 클릭 이벤트 연결
	if (IsValid(Button_ExitGame))
	{
		Button_ExitGame->OnClicked.RemoveDynamic(this,&USGGameResultWidget::OnExitGameClicked);
		Button_ExitGame->OnClicked.AddDynamic(this, &USGGameResultWidget::OnExitGameClicked);
	}
	
	if (Button_Rematch)
	{
		Button_Rematch->OnClicked.AddDynamic(this, &USGGameResultWidget::OnRematchClicked);
	}
	
	// 최종 점수 및 승리 메세지 세팅
	if (ASGMainGameState* GS = Cast<ASGMainGameState>(UGameplayStatics::GetGameState(this)))
	{
		int32 FinalBlueTeamScore = GS->BlueTeamScore;
		int32 FinalRedTeamScore = GS->RedTeamScore;
		
		// 최종 점수 세팅
		if (Text_BlueTeamScore)
		{
			Text_BlueTeamScore->SetText(FText::AsNumber(FinalBlueTeamScore));
		}
		if (Text_RedTeamScore)
		{
			Text_RedTeamScore->SetText(FText::AsNumber(FinalRedTeamScore));
		}
		
		if (IsValid(Text_WinMessage))
		{
			if (FinalRedTeamScore > FinalBlueTeamScore)
			{
				Text_WinMessage->SetText(FText::FromString("RED TEAM WINS!"));
				Text_WinMessage->SetColorAndOpacity(FSlateColor(FLinearColor::Red));
			}
			else if (FinalBlueTeamScore > FinalRedTeamScore)
			{
				Text_WinMessage->SetText(FText::FromString("BLUE TEAM WINS!"));
				Text_WinMessage->SetColorAndOpacity(FSlateColor(FLinearColor::Blue));
			}
			else
			{
				{
					Text_WinMessage->SetText(FText::FromString("DRAW!"));
					Text_WinMessage->SetColorAndOpacity(FSlateColor(FLinearColor::Gray));
				}
			}
		}
	}
}

void USGGameResultWidget::NativeDestruct()
{
	if (IsValid(Button_ExitGame))
	{
		Button_ExitGame->OnClicked.RemoveDynamic(
			this,
			&USGGameResultWidget::OnExitGameClicked
		);
	}
	Super::NativeDestruct();
}

void USGGameResultWidget::OnExitGameClicked()
{
	ASGMainPlayerController* MainPlayerController =Cast<ASGMainPlayerController>(GetOwningPlayer());
	
	if (!IsValid(MainPlayerController))
	{
		return;
	}
	if (IsValid(Button_ExitGame))
	{
		Button_ExitGame->SetIsEnabled(false);
	}
	MainPlayerController->RequestReturnToMainMenu();
}

void USGGameResultWidget::OnRematchClicked()
{
	if (Button_Rematch)
	{
		Button_Rematch->SetIsEnabled(false);
	}
	
	// TODO: PlayerController쪽 리매치 request 로직 가져오기
}


