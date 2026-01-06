// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UI/Widget/AO_UserWidget.h"
#include "AO_ConfirmReturnToMenuWidget.generated.h"

class UButton;
class UTextBlock;

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnConfirmLeaveToMenu);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnCancelLeaveToMenu);
/**
 * 
 */
UCLASS()
class AO_API UAO_ConfirmReturnToMenuWidget : public UAO_UserWidget
{
	GENERATED_BODY()
	
public:
	virtual void NativeOnInitialized() override;
	virtual FReply NativeOnPreviewKeyDown(const FGeometry& InGeometry, const FKeyEvent& InKeyEvent) override;

public:
	UPROPERTY(BlueprintAssignable, Category="Confirm")
	FOnConfirmLeaveToMenu OnConfirmLeaveToMenu;

	UPROPERTY(BlueprintAssignable, Category="Confirm")
	FOnCancelLeaveToMenu OnCancelLeaveToMenu;

protected:
	/* WBP Application Btn 으로 수정 (JM)
	 *UPROPERTY(meta = (BindWidget))
	UButton* Btn_Confirm;

	UPROPERTY(meta = (BindWidget))
	UButton* Btn_Cancel;

	UPROPERTY(meta = (BindWidget, OptionalWidget = true))
	UTextBlock* Txt_Message; */

protected:
	UFUNCTION(BlueprintCallable, meta = (BindWidget))
	void HandleClicked_Confirm();

	UFUNCTION(BlueprintCallable, meta = (BindWidget))
	void HandleClicked_Cancel();
};
