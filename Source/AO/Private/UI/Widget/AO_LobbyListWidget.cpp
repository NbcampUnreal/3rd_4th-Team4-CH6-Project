// Fill out your copyright notice in the Description page of Project Settings.

#include "UI/Widget/AO_LobbyListWidget.h"

#include "UI/Widget/AO_MainMenuWidget.h"
#include "UI/Widget/AO_LobbyListEntryWidget.h"
#include "UI/Widget/AO_PasswordDialogWidget.h"

#include "Online/AO_OnlineSessionSubsystem.h"

#include "Components/Button.h"
#include "Components/ScrollBox.h"
#include "Components/EditableTextBox.h"
#include "Components/TextBlock.h"
#include "Kismet/GameplayStatics.h"

UAO_OnlineSessionSubsystem* UAO_LobbyListWidget::GetSub() const
{
	if(const UGameInstance* GI = GetGameInstance())
	{
		return GI->GetSubsystem<UAO_OnlineSessionSubsystem>();
	}
	return nullptr;
}

void UAO_LobbyListWidget::NativeConstruct()
{
	Super::NativeConstruct();

	if(Btn_Refresh)
	{
		Btn_Refresh->OnClicked.AddDynamic(this, &ThisClass::OnClicked_Refresh);
	}
	if(Btn_Close)
	{
		Btn_Close->OnClicked.AddDynamic(this, &ThisClass::OnClicked_Close);
	}

	if(Edt_Search)
	{
		Edt_Search->OnTextCommitted.AddDynamic(this, &ThisClass::OnSearchTextCommitted);
	}
	if(Btn_Prev)
	{
		Btn_Prev->OnClicked.AddDynamic(this, &ThisClass::OnClicked_PrevPage);
	}
	if(Btn_Next)
	{
		Btn_Next->OnClicked.AddDynamic(this, &ThisClass::OnClicked_NextPage);
	}

	if(UAO_OnlineSessionSubsystem* Sub = GetSub())
	{
		Sub->OnFindSessionsCompleteEvent.AddDynamic(this, &ThisClass::OnFindSessionsComplete);
	}

	OnClicked_Refresh();
}

void UAO_LobbyListWidget::NativeDestruct()
{
	if(UAO_OnlineSessionSubsystem* Sub = GetSub())
	{
		Sub->OnFindSessionsCompleteEvent.RemoveDynamic(this, &ThisClass::OnFindSessionsComplete);
	}
	Super::NativeDestruct();
}

void UAO_LobbyListWidget::SetParentMenu(UAO_MainMenuWidget* InParent)
{
	ParentMenu = InParent;
}

void UAO_LobbyListWidget::OnClicked_Refresh()
{
	PageIndex = 0;
	CurrentSearch.Reset();

	if(Edt_Search)
	{
		Edt_Search->SetText(FText::GetEmpty());
	}

	if(UAO_OnlineSessionSubsystem* Sub = GetSub())
	{
		Sub->FindSessionsAuto(50);
	}
}

void UAO_LobbyListWidget::OnClicked_Close()
{
	if (ParentMenu)
	{
		ParentMenu->SetVisibility(ESlateVisibility::Visible);

		if (APlayerController* PC = GetOwningPlayer())
		{
			FInputModeUIOnly Mode;
			Mode.SetWidgetToFocus(ParentMenu->TakeWidget());
			Mode.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);
			PC->SetInputMode(Mode);
			PC->bShowMouseCursor = true;
		}
	}
	else
	{
		if (APlayerController* PC = GetOwningPlayer())
		{
			if (MainMenuClass)
			{
				if (UAO_MainMenuWidget* NewMenu = CreateWidget<UAO_MainMenuWidget>(PC, MainMenuClass))
				{
					NewMenu->AddToViewport(100);
					FInputModeUIOnly Mode;
					Mode.SetWidgetToFocus(NewMenu->TakeWidget());
					Mode.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);
					PC->SetInputMode(Mode);
					PC->bShowMouseCursor = true;
				}
			}
		}
	}
	RemoveFromParent();
}

void UAO_LobbyListWidget::OnFindSessionsComplete(bool /*bSuccessful*/)
{
	PageIndex = 0;
	RebuildFilter();
	RebuildList();
}

void UAO_LobbyListWidget::OnSearchTextCommitted(const FText& Text, ETextCommit::Type /*CommitMethod*/)
{
	CurrentSearch = Text.ToString();
	PageIndex = 0;
	RebuildFilter();
	RebuildList();
}

void UAO_LobbyListWidget::OnClicked_PrevPage()
{
	if(PageIndex > 0)
	{
		--PageIndex;
		RebuildList();
	}
}

void UAO_LobbyListWidget::OnClicked_NextPage()
{
	const int32 TotalPages = GetTotalPages();
	if(PageIndex + 1 < TotalPages)
	{
		++PageIndex;
		RebuildList();
	}
}

void UAO_LobbyListWidget::RebuildFilter()
{
	FilteredIndices.Reset();

	if(const UAO_OnlineSessionSubsystem* Sub = GetSub())
	{
		const int32 Count = Sub->GetNumSearchResults();

		if(CurrentSearch.IsEmpty())
		{
			FilteredIndices.Reserve(Count);
			for(int32 i = 0; i < Count; ++i)
			{
				FilteredIndices.Add(i);
			}
		}
		else
		{
			const FString Query = CurrentSearch.ToLower();
			for(int32 i = 0; i < Count; ++i)
			{
				const FString Name = Sub->GetServerNameByIndex(i).ToLower();
				if(Name.Contains(Query))
				{
					FilteredIndices.Add(i);
				}
			}
		}
	}

	ClampAndUpdatePage();
	UpdatePageUI();
}

void UAO_LobbyListWidget::ClampAndUpdatePage()
{
	const int32 TotalPages = GetTotalPages();

	if(TotalPages <= 0)
	{
		PageIndex = 0;
		return;
	}
	if(PageIndex >= TotalPages)
	{
		PageIndex = TotalPages - 1;
	}
}

int32 UAO_LobbyListWidget::GetTotalPages() const
{
	const int32 N = FilteredIndices.Num();
	if(N <= 0)
	{
		return 0;
	}
	return (N + kItemsPerPage - 1) / kItemsPerPage;
}

void UAO_LobbyListWidget::UpdatePageUI()
{
	if(!Txt_PageInfo)
	{
		return;
	}

	const int32 TotalPages = GetTotalPages();
	if(TotalPages <= 0)
	{
		Txt_PageInfo->SetText(FText::FromString(TEXT("0 / 0")));
		return;
	}

	Txt_PageInfo->SetText(FText::FromString(FString::Printf(TEXT("%d / %d"), PageIndex + 1, TotalPages)));
}

void UAO_LobbyListWidget::RebuildList()
{
	if(!Scroll_SessionList)
	{
		return;
	}

	Scroll_SessionList->ClearChildren();

	if(FilteredIndices.Num() == 0 && GetSub())
	{
		RebuildFilter();
	}

	if(UAO_OnlineSessionSubsystem* Sub = GetSub())
	{
		const int32 Start = PageIndex * kItemsPerPage;
		const int32 EndExclusive = FMath::Min(Start + kItemsPerPage, FilteredIndices.Num());

		for(int32 k = Start; k < EndExclusive; ++k)
		{
			const int32 ResultIndex = FilteredIndices[k];

			if(!LobbyEntryClass)
			{
				continue;
			}

			if(UAO_LobbyListEntryWidget* Entry = CreateWidget<UAO_LobbyListEntryWidget>(GetOwningPlayer(), LobbyEntryClass))
			{
				Entry->SetParentLobby(this);

				const FString RoomName = Sub->GetServerNameByIndex(ResultIndex);
				const int32 OpenSlots = Sub->GetOpenPublicConnectionsByIndex(ResultIndex);
				const int32 MaxSlots  = Sub->GetMaxPublicConnectionsByIndex(ResultIndex);
				const bool  bHasPw    = Sub->IsPasswordRequiredByIndex(ResultIndex);

				Entry->Setup(ResultIndex, RoomName, OpenSlots, MaxSlots, bHasPw);
				Scroll_SessionList->AddChild(Entry);
			}
		}
	}

	UpdatePageUI();
}

void UAO_LobbyListWidget::HandleJoin(int32 Index, bool bNeedsPassword)
{
	if(!bNeedsPassword)
	{
		if(UAO_OnlineSessionSubsystem* Sub = GetSub())
		{
			Sub->JoinSessionByIndex(Index);
		}
		return;
	}
	
	if(!PasswordDialogClass)
	{
		UE_LOG(LogTemp, Warning, TEXT("PasswordDialogClass not set."));
		return;
	}

	if(UAO_PasswordDialogWidget* Dialog = CreateWidget<UAO_PasswordDialogWidget>(GetOwningPlayer(), PasswordDialogClass))
	{
		Dialog->SetParentList(this);
		Dialog->SetTargetIndex(Index);
		Dialog->AddToViewport(200);

		SetVisibility(ESlateVisibility::Collapsed);

		if(APlayerController* PC = GetOwningPlayer())
		{
			FInputModeUIOnly Mode;
			Mode.SetWidgetToFocus(Dialog->TakeWidget());
			Mode.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);
			PC->SetInputMode(Mode);
			PC->bShowMouseCursor = true;
		}
	}
}
