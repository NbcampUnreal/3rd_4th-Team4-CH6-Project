// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/Widget/AO_ConfirmReturnToMenuWidget.h"

#include "Components/Button.h"
#include "Components/TextBlock.h"

void UAO_ConfirmReturnToMenuWidget::NativeOnInitialized()
{
	Super::NativeOnInitialized();

	if (Btn_Confirm != nullptr)
	{
		Btn_Confirm->OnClicked.AddDynamic(this, &UAO_ConfirmReturnToMenuWidget::HandleClicked_Confirm);
	}

	if (Btn_Cancel != nullptr)
	{
		Btn_Cancel->OnClicked.AddDynamic(this, &UAO_ConfirmReturnToMenuWidget::HandleClicked_Cancel);
	}

	if (Txt_Message != nullptr)
	{
		Txt_Message->SetText(FText::FromString(TEXT("Are you sure you want to return to the main menu?")));
	}
}

void UAO_ConfirmReturnToMenuWidget::HandleClicked_Confirm()
{
	OnConfirmLeaveToMenu.Broadcast();
}

void UAO_ConfirmReturnToMenuWidget::HandleClicked_Cancel()
{
	OnCancelLeaveToMenu.Broadcast();
}
