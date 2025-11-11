// Fill out your copyright notice in the Description page of Project Settings.

#include "UI/Widget/AO_HostDialogWidget.h"

#include "Online/AO_OnlineSessionSubsystem.h"
#include "Components/Button.h"
#include "Components/EditableTextBox.h"
#include "Blueprint/WidgetBlueprintLibrary.h"
#include "Kismet/GameplayStatics.h"
#include "AO/AO_Log.h"

void UAO_HostDialogWidget::NativeConstruct()
{
	Super::NativeConstruct();

	if (Btn_Create && !Btn_Create->OnClicked.IsAlreadyBound(this, &ThisClass::OnClicked_Create))
	{
		Btn_Create->OnClicked.AddDynamic(this, &ThisClass::OnClicked_Create);
	}
	if (Btn_Cancel && !Btn_Cancel->OnClicked.IsAlreadyBound(this, &ThisClass::OnClicked_Cancel))
	{
		Btn_Cancel->OnClicked.AddDynamic(this, &ThisClass::OnClicked_Cancel);
	}
	if (Btn_Backdrop && !Btn_Backdrop->OnClicked.IsAlreadyBound(this, &ThisClass::OnClicked_Backdrop))
	{
		Btn_Backdrop->OnClicked.AddDynamic(this, &ThisClass::OnClicked_Backdrop);
	}
	
	ApplyModalInput();

	AO_LOG(LogJSH, Log, TEXT("HostDialog shown (mouse-only modal)"));
}

void UAO_HostDialogWidget::ApplyModalInput()
{
	if (APlayerController* PC = UGameplayStatics::GetPlayerController(GetWorld(), 0))
	{
		FInputModeUIOnly Mode;
		Mode.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);
		PC->SetInputMode(Mode);
		PC->bShowMouseCursor = true;
	}
}

void UAO_HostDialogWidget::OnClicked_Create()
{
	const FString RoomName = Txt_RoomName ? Txt_RoomName->GetText().ToString() : TEXT("New Room");
	const FString Password = Txt_Password ? Txt_Password->GetText().ToString() : TEXT("");

	AO_LOG(LogJSH, Log, TEXT("Create clicked: Room='%s' PwLen=%d"), *RoomName, Password.Len());

	if (UGameInstance* GI = GetGameInstance())
	{
		if (UAO_OnlineSessionSubsystem* Sub = GI->GetSubsystem<UAO_OnlineSessionSubsystem>())
		{
			Sub->HostSessionAuto(4, RoomName, Password);
		}
	}

	RemoveFromParent();
}

void UAO_HostDialogWidget::OnClicked_Cancel()
{
	AO_LOG(LogJSH, Log, TEXT("Cancel clicked"));
	RemoveFromParent();
}

void UAO_HostDialogWidget::OnClicked_Backdrop()
{
	OnClicked_Cancel();
}
