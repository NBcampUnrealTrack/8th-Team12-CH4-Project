// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/SGChangeUsernameWidget.h"

#include "SoccerGame/Public/PlayerController/SGLobbyPlayerController.h"
#include "Components/Button.h"
#include "Components/EditableTextBox.h"
#include "Components/TextBlock.h"

void USGChangeUsernameWidget::NativeConstruct()
{
	Super::NativeConstruct();
	
	
	
	if (IsValid(Button_Ok))
	{
		Button_Ok->OnClicked.AddDynamic(this,&USGChangeUsernameWidget::OnClickedOkButton);
	}

	if (IsValid(Button_Cancel))
	{
		Button_Cancel->OnClicked.AddDynamic(this,&USGChangeUsernameWidget::OnClickedCancelButton);
	}

	if (IsValid(EditableText_Input))
	{
		EditableText_Input->OnTextCommitted.AddDynamic(this,&USGChangeUsernameWidget::OnUsernameTextCommitted);
		EditableText_Input->SetKeyboardFocus();
	}
	ASGLobbyPlayerController* LobbyPC = Cast<ASGLobbyPlayerController>(GetOwningPlayer());
	ASGLobbyPlayerState* LobbyPS = Cast<ASGLobbyPlayerState>(LobbyPC->PlayerState);
	if (IsValid(LobbyPC) && IsValid(LobbyPS))
	{
		FString CurrentUsername = LobbyPS->GetCustomPlayerName();
		FString CurrentUsernameText = FString::Printf(TEXT("Current Username: %s"), *CurrentUsername);
		Text_CurrentUsername->SetText(FText::FromString(CurrentUsernameText));
	}
	
}

void USGChangeUsernameWidget::NativeDestruct()
{
	Super::NativeDestruct();
	if (IsValid(Button_Ok))
	{
		Button_Ok->OnClicked.RemoveDynamic(this,&USGChangeUsernameWidget::OnClickedOkButton);
	}

	if (IsValid(Button_Cancel))
	{
		Button_Cancel->OnClicked.RemoveDynamic(this,&USGChangeUsernameWidget::OnClickedCancelButton);
	}

	if (IsValid(EditableText_Input))
	{
		EditableText_Input->OnTextCommitted.RemoveDynamic(this,&USGChangeUsernameWidget::OnUsernameTextCommitted);
	}

}
void USGChangeUsernameWidget::OnClickedOkButton()
{
	// 확인 버튼을 눌렀을 때 입력한 사용자 이름을 검사하고, 
	// 문제가 없으면 이름 변경을 적용하는 코드
	
	if (!IsValid(EditableText_Input))
	{
		return;
	}

	const FString NewUsername =EditableText_Input->GetText().ToString().TrimStartAndEnd();

	if (!IsValidUsername(NewUsername))
	{
		UE_LOG(LogTemp,Warning,TEXT("Invalid username: %s"),*NewUsername);

		return;
	}

	// 입력된 text 가 맞다면 ApplyUserName함수로 이동
	ApplyUsername(NewUsername);
}

void USGChangeUsernameWidget::OnClickedCancelButton()
{
	ASGLobbyPlayerController* LobbyPlayerController =GetOwningPlayer<ASGLobbyPlayerController>();

	if (IsValid(LobbyPlayerController))
	{
		LobbyPlayerController->CloseChangeUsernameWidget();

		return;
	}
	RemoveFromParent();

}

void USGChangeUsernameWidget::OnUsernameTextCommitted(const FText& Text, ETextCommit::Type CommitMethod)
{
	// EnterKiey를 눌렀을 때 허용하는 코드 
	if (CommitMethod != ETextCommit::OnEnter)
	{
		return;
	}

	const FString NewUsername =Text.ToString().TrimStartAndEnd();

	if (!IsValidUsername(NewUsername))
	{
		return;
	}

	ApplyUsername(NewUsername);
}

bool USGChangeUsernameWidget::IsValidUsername(const FString& Username) const
{
	if (Username.IsEmpty())
	{
		return false;
	}

	// 최소 이름 길이  ( 매직넘어 설정 ) 
	int32 MinUsernameLength = 2;
	// 최대 이름 길이 ( 매직넘버 설정 ) 
	int32 MaxUsernameLength = 16;

	return Username.Len() >= MinUsernameLength
		&& Username.Len() <= MaxUsernameLength;
}
void USGChangeUsernameWidget::ApplyUsername(const FString& NewUsername)
{
	UE_LOG(LogTemp,Log,TEXT("Apply new username: %s"),*NewUsername);
	
	ASGLobbyPlayerController* LobbyPC =GetOwningPlayer<ASGLobbyPlayerController>();
	
	if (!IsValid(LobbyPC))
	{
		UE_LOG(LogTemp,Error,TEXT("None PlayerController"));
		return;
	}

	// TODO:
	// PlayerController 또는 PlayerState에 서버 RPC를 호출하여
	// 실제 플레이어 이름을 변경합니다.
	LobbyPC->RequestChangeUsername(NewUsername);
	LobbyPC->CloseChangeUsernameWidget();
}
