// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "SGChangeUsernameWidget.generated.h"

/**
 * 
 */

class UButton;
class UTextBlock;
class UEditableTextBox;
UCLASS()
class SOCCERGAME_API USGChangeUsernameWidget : public UUserWidget
{
	GENERATED_BODY()
	
protected:
	virtual void NativeConstruct() override;
	virtual void NativeDestruct() override;

private:
	/**
	 * 블루프린트 위젯 이름과 정확히 일치해야 합니다.
	 */
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UEditableTextBox> EditableText_Input;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UButton> Button_Ok;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UButton> Button_Cancel;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> Text_CurrentUsername;
	
private:
	UFUNCTION()
	void OnClickedOkButton();

	UFUNCTION()
	void OnClickedCancelButton();

	UFUNCTION()
	void OnUsernameTextCommitted(
		const FText& Text,
		ETextCommit::Type CommitMethod
	);

private:
	bool IsValidUsername(const FString& Username) const;

	void ApplyUsername(const FString& NewUsername);
};
