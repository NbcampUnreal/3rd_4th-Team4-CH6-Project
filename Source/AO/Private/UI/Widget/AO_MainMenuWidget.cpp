// Fill out your copyright notice in the Description page of Project Settings.

#include "UI/Widget/AO_MainMenuWidget.h"

#include "UI/Widget/AO_HostDialogWidget.h"
#include "UI/Widget/AO_LobbyListWidget.h"

#include "Online/AO_OnlineSessionSubsystem.h"
#include "Components/Button.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetSystemLibrary.h"
#include "AO/AO_Log.h"

void UAO_MainMenuWidget::NativeConstruct()
{
	Super::NativeConstruct();

	if (Btn_Host)
	{
		Btn_Host->OnClicked.AddDynamic(this, &ThisClass::OnClicked_Host);
	}
	else
	{
		AO_LOG(LogJSH, Warning, TEXT("NativeConstruct: Btn_Host is null"));
	}

	if (Btn_Join)
	{
		Btn_Join->OnClicked.AddDynamic(this, &ThisClass::OnClicked_Join);
	}
	else
	{
		AO_LOG(LogJSH, Warning, TEXT("NativeConstruct: Btn_Join is null"));
	}

	if (Btn_Settings)
	{
		Btn_Settings->OnClicked.AddDynamic(this, &ThisClass::OnClicked_Settings);
	}
	else
	{
		AO_LOG(LogJSH, Warning, TEXT("NativeConstruct: Btn_Settings is null"));
	}

	if (Btn_Quit)
	{
		Btn_Quit->OnClicked.AddDynamic(this, &ThisClass::OnClicked_Quit);
	}
	else
	{
		AO_LOG(LogJSH, Warning, TEXT("NativeConstruct: Btn_Quit is null"));
	}
}

void UAO_MainMenuWidget::OnClicked_Host()
{
	AO_LOG(LogJSH, Log, TEXT("Host clicked"));

	if (!HostDialogClass)
	{
		AO_LOG(LogJSH, Error, TEXT("OnClicked_Host: HostDialogClass not set"));
		return;
	}

	if (UAO_HostDialogWidget* Dialog = CreateWidget<UAO_HostDialogWidget>(GetWorld(), HostDialogClass))
	{
		Dialog->AddToViewport(200);
		Dialog->SetVisibility(ESlateVisibility::Visible);
		Dialog->SetIsFocusable(true);
		AO_LOG(LogJSH, Log, TEXT("OnClicked_Host: HostDialog popup displayed"));
	}
	else
	{
		AO_LOG(LogJSH, Error, TEXT("OnClicked_Host: CreateWidget(UAO_HostDialogWidget) failed"));
	}
}

void UAO_MainMenuWidget::OnClicked_Join()
{
	AO_LOG(LogJSH, Log, TEXT("Join clicked"));

	if (UGameInstance* GI = GetGameInstance())
	{
		if (UAO_OnlineSessionSubsystem* Subsystem = GI->GetSubsystem<UAO_OnlineSessionSubsystem>())
		{
			AO_LOG(LogJSH, Log, TEXT("OnClicked_Join: Starting session search (Auto)"));
			Subsystem->FindSessionsAuto(50);
		}
		else
		{
			AO_LOG(LogJSH, Warning, TEXT("OnClicked_Join: OnlineSessionSubsystem is null"));
		}
	}
	else
	{
		AO_LOG(LogJSH, Warning, TEXT("OnClicked_Join: GameInstance is null"));
	}

	if (LobbyListWidgetClass)
	{
		if (UAO_LobbyListWidget* Widget = CreateWidget<UAO_LobbyListWidget>(GetWorld(), LobbyListWidgetClass))
		{
			AO_LOG(LogJSH, Log, TEXT("OnClicked_Join: LobbyList created: %s"), *Widget->GetName());

			Widget->SetParentMenu(this);
			Widget->AddToViewport(100);
			Widget->SetVisibility(ESlateVisibility::Visible);
			Widget->SetIsFocusable(true);

			if (APlayerController* PC = UGameplayStatics::GetPlayerController(GetWorld(), 0))
			{
				FInputModeUIOnly Mode;
				Mode.SetWidgetToFocus(Widget->TakeWidget());
				Mode.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);
				PC->SetInputMode(Mode);
				PC->bShowMouseCursor = true;

				AO_LOG(LogJSH, Log, TEXT("OnClicked_Join: Focus moved to LobbyList widget"));
			}
			else
			{
				AO_LOG(LogJSH, Warning, TEXT("OnClicked_Join: PlayerController is null (cannot set input mode)"));
			}

			SetVisibility(ESlateVisibility::Collapsed);
		}
		else
		{
			AO_LOG(LogJSH, Error, TEXT("OnClicked_Join: CreateWidget(LobbyList) failed"));
		}
	}
	else
	{
		AO_LOG(LogJSH, Warning, TEXT("OnClicked_Join: LobbyListWidgetClass not set"));
	}
}

void UAO_MainMenuWidget::OnClicked_Settings()
{
	AO_LOG(LogJSH, Log, TEXT("Settings clicked (placeholder)"));
	// TODO: 설정 위젯 추가 예정
}

void UAO_MainMenuWidget::OnClicked_Quit()
{
	if (APlayerController* PC = UGameplayStatics::GetPlayerController(GetWorld(), 0))
	{
		AO_LOG(LogJSH, Log, TEXT("Quit clicked"));
		UKismetSystemLibrary::QuitGame(GetWorld(), PC, EQuitPreference::Quit, false);
	}
	else
	{
		AO_LOG(LogJSH, Warning, TEXT("OnClicked_Quit: PlayerController is null, QuitGame skipped"));
	}
}
