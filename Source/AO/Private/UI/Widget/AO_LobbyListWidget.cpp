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
#include "AO/AO_Log.h"

UAO_OnlineSessionSubsystem* UAO_LobbyListWidget::GetSub() const
{
	if(const UGameInstance* GI = GetGameInstance())
	{
		if(UAO_OnlineSessionSubsystem* Sub = GI->GetSubsystem<UAO_OnlineSessionSubsystem>())
		{
			return Sub;
		}

		AO_LOG(LogJSH, Warning, TEXT("GetSub: OnlineSessionSubsystem is null"));
		return nullptr;
	}

	AO_LOG(LogJSH, Warning, TEXT("GetSub: GameInstance is null"));
	return nullptr;
}

void UAO_LobbyListWidget::NativeConstruct()
{
	Super::NativeConstruct();

	if(Btn_Refresh)
	{
		Btn_Refresh->OnClicked.AddDynamic(this, &ThisClass::OnClicked_Refresh);
	}
	else
	{
		AO_LOG(LogJSH, Warning, TEXT("NativeConstruct: Btn_Refresh is null"));
	}

	if(Btn_Close)
	{
		Btn_Close->OnClicked.AddDynamic(this, &ThisClass::OnClicked_Close);
	}
	else
	{
		AO_LOG(LogJSH, Warning, TEXT("NativeConstruct: Btn_Close is null"));
	}

	if(Edt_Search)
	{
		Edt_Search->OnTextCommitted.AddDynamic(this, &ThisClass::OnSearchTextCommitted);
	}
	else
	{
		AO_LOG(LogJSH, Warning, TEXT("NativeConstruct: Edt_Search is null"));
	}

	if(Btn_Prev)
	{
		Btn_Prev->OnClicked.AddDynamic(this, &ThisClass::OnClicked_PrevPage);
	}
	else
	{
		AO_LOG(LogJSH, Warning, TEXT("NativeConstruct: Btn_Prev is null"));
	}

	if(Btn_Next)
	{
		Btn_Next->OnClicked.AddDynamic(this, &ThisClass::OnClicked_NextPage);
	}
	else
	{
		AO_LOG(LogJSH, Warning, TEXT("NativeConstruct: Btn_Next is null"));
	}

	if(UAO_OnlineSessionSubsystem* Sub = GetSub())
	{
		Sub->OnFindSessionsCompleteEvent.AddDynamic(this, &ThisClass::OnFindSessionsComplete);
	}
	else
	{
		AO_LOG(LogJSH, Warning, TEXT("NativeConstruct: Failed to bind OnFindSessionsCompleteEvent (Sub is null)"));
	}

	AO_LOG(LogJSH, Log, TEXT("NativeConstruct: LobbyList initialized, requesting initial refresh"));
	OnClicked_Refresh();
}

void UAO_LobbyListWidget::NativeDestruct()
{
	if(UAO_OnlineSessionSubsystem* Sub = GetSub())
	{
		Sub->OnFindSessionsCompleteEvent.RemoveDynamic(this, &ThisClass::OnFindSessionsComplete);
	}
	else
	{
		AO_LOG(LogJSH, Warning, TEXT("NativeDestruct: Sub is null, cannot RemoveDynamic"));
	}

	Super::NativeDestruct();
}

void UAO_LobbyListWidget::SetParentMenu(UAO_MainMenuWidget* InParent)
{
	ParentMenu = InParent;
}

void UAO_LobbyListWidget::OnClicked_Refresh()
{
	AO_LOG(LogJSH, Log, TEXT("OnClicked_Refresh: Resetting page and search"));

	PageIndex = 0;
	CurrentSearch.Reset();

	if(Edt_Search)
	{
		Edt_Search->SetText(FText::GetEmpty());
	}
	else
	{
		AO_LOG(LogJSH, Warning, TEXT("OnClicked_Refresh: Edt_Search is null, cannot clear text"));
	}

	if(UAO_OnlineSessionSubsystem* Sub = GetSub())
	{
		AO_LOG(LogJSH, Log, TEXT("OnClicked_Refresh: FindSessionsAuto(50)"));
		Sub->FindSessionsAuto(50);
	}
	else
	{
		AO_LOG(LogJSH, Warning, TEXT("OnClicked_Refresh: Sub is null, cannot FindSessions"));
	}
}

void UAO_LobbyListWidget::OnClicked_Close()
{
	AO_LOG(LogJSH, Log, TEXT("OnClicked_Close: Closing LobbyList"));

	if(ParentMenu)
	{
		ParentMenu->SetVisibility(ESlateVisibility::Visible);

		if(APlayerController* PC = GetOwningPlayer())
		{
			FInputModeUIOnly Mode;
			Mode.SetWidgetToFocus(ParentMenu->TakeWidget());
			Mode.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);
			PC->SetInputMode(Mode);
			PC->bShowMouseCursor = true;

			AO_LOG(LogJSH, Log, TEXT("OnClicked_Close: Returned focus to ParentMenu"));
		}
		else
		{
			AO_LOG(LogJSH, Warning, TEXT("OnClicked_Close: OwningPlayer is null (ParentMenu path)"));
		}
	}
	else
	{
		if(APlayerController* PC = GetOwningPlayer())
		{
			if(MainMenuClass)
			{
				if(UAO_MainMenuWidget* NewMenu = CreateWidget<UAO_MainMenuWidget>(PC, MainMenuClass))
				{
					NewMenu->AddToViewport(100);

					FInputModeUIOnly Mode;
					Mode.SetWidgetToFocus(NewMenu->TakeWidget());
					Mode.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);
					PC->SetInputMode(Mode);
					PC->bShowMouseCursor = true;

					AO_LOG(LogJSH, Log, TEXT("OnClicked_Close: Created new MainMenu widget and moved focus"));
				}
				else
				{
					AO_LOG(LogJSH, Error, TEXT("OnClicked_Close: CreateWidget(MainMenu) failed"));
				}
			}
			else
			{
				AO_LOG(LogJSH, Warning, TEXT("OnClicked_Close: MainMenuClass not set"));
			}
		}
		else
		{
			AO_LOG(LogJSH, Warning, TEXT("OnClicked_Close: OwningPlayer is null (no ParentMenu path)"));
		}
	}

	RemoveFromParent();
}

void UAO_LobbyListWidget::OnFindSessionsComplete(bool /*bSuccessful*/)
{
	AO_LOG(LogJSH, Log, TEXT("OnFindSessionsComplete: Rebuilding filter and list"));

	PageIndex = 0;
	RebuildFilter();
	RebuildList();
}

void UAO_LobbyListWidget::OnSearchTextCommitted(const FText& Text, ETextCommit::Type /*CommitMethod*/)
{
	CurrentSearch = Text.ToString();
	PageIndex = 0;

	AO_LOG(LogJSH, Log, TEXT("OnSearchTextCommitted: Query=\"%s\""), *CurrentSearch);

	RebuildFilter();
	RebuildList();
}

void UAO_LobbyListWidget::OnClicked_PrevPage()
{
	if(PageIndex > 0)
	{
		--PageIndex;
		AO_LOG(LogJSH, Log, TEXT("OnClicked_PrevPage: PageIndex -> %d"), PageIndex);
		RebuildList();
	}
	else
	{
		AO_LOG(LogJSH, Log, TEXT("OnClicked_PrevPage: Already at first page"));
	}
}

void UAO_LobbyListWidget::OnClicked_NextPage()
{
	const int32 TotalPages = GetTotalPages();
	if(PageIndex + 1 < TotalPages)
	{
		++PageIndex;
		AO_LOG(LogJSH, Log, TEXT("OnClicked_NextPage: PageIndex -> %d / %d"), PageIndex + 1, TotalPages);
		RebuildList();
	}
	else
	{
		AO_LOG(LogJSH, Log, TEXT("OnClicked_NextPage: Already at last page (Page=%d / %d)"), PageIndex + 1, TotalPages);
	}
}

void UAO_LobbyListWidget::RebuildFilter()
{
	FilteredIndices.Reset();

	if(const UAO_OnlineSessionSubsystem* Sub = GetSub())
	{
		const int32 Count = Sub->GetNumSearchResults();
		AO_LOG(LogJSH, Log, TEXT("RebuildFilter: TotalSearchResults=%d, Query=\"%s\""),
			Count, *CurrentSearch);

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
	else
	{
		AO_LOG(LogJSH, Warning, TEXT("RebuildFilter: Sub is null, no sessions to filter"));
	}

	AO_LOG(LogJSH, Log, TEXT("RebuildFilter: FilteredCount=%d"), FilteredIndices.Num());

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
	return (N + NumSessionsPerPage - 1) / NumSessionsPerPage;
}

void UAO_LobbyListWidget::UpdatePageUI()
{
	if(!Txt_PageInfo)
	{
		AO_LOG(LogJSH, Warning, TEXT("UpdatePageUI: Txt_PageInfo is null"));
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
		AO_LOG(LogJSH, Warning, TEXT("RebuildList: Scroll_SessionList is null"));
		return;
	}

	Scroll_SessionList->ClearChildren();

	if(FilteredIndices.Num() == 0 && GetSub())
	{
		AO_LOG(LogJSH, Log, TEXT("RebuildList: FilteredIndices empty, rebuilding filter"));
		RebuildFilter();
	}

	if(UAO_OnlineSessionSubsystem* Sub = GetSub())
	{
		const int32 Start = PageIndex * NumSessionsPerPage;
		const int32 EndExclusive = FMath::Min(Start + NumSessionsPerPage, FilteredIndices.Num());

		AO_LOG(LogJSH, Log, TEXT("RebuildList: PageIndex=%d, Showing=%d..%d (FilteredTotal=%d)"),
			PageIndex, Start, EndExclusive - 1, FilteredIndices.Num());

		for(int32 k = Start; k < EndExclusive; ++k)
		{
			const int32 ResultIndex = FilteredIndices[k];

			if(!LobbyEntryClass)
			{
				AO_LOG(LogJSH, Warning, TEXT("RebuildList: LobbyEntryClass not set, skipping entry"));
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
			else
			{
				AO_LOG(LogJSH, Error, TEXT("RebuildList: CreateWidget(LobbyListEntry) failed (Index=%d)"), ResultIndex);
			}
		}
	}
	else
	{
		AO_LOG(LogJSH, Warning, TEXT("RebuildList: Sub is null, cannot build list"));
	}

	UpdatePageUI();
}

void UAO_LobbyListWidget::HandleJoin(int32 Index, bool bNeedsPassword)
{
	AO_LOG(LogJSH, Log, TEXT("HandleJoin: Index=%d, NeedsPassword=%d"),
		Index, static_cast<int32>(bNeedsPassword));

	if(!bNeedsPassword)
	{
		if(UAO_OnlineSessionSubsystem* Sub = GetSub())
		{
			Sub->JoinSessionByIndex(Index);
		}
		else
		{
			AO_LOG(LogJSH, Warning, TEXT("HandleJoin: Sub is null, cannot JoinSessionByIndex"));
		}
		return;
	}
	
	if(!PasswordDialogClass)
	{
		AO_LOG(LogJSH, Warning, TEXT("HandleJoin: PasswordDialogClass not set"));
		return;
	}

	if(UAO_PasswordDialogWidget* Dialog = CreateWidget<UAO_PasswordDialogWidget>(GetOwningPlayer(), PasswordDialogClass))
	{
		Dialog->SetParentList(this);
		Dialog->SetTargetIndex(Index);
		Dialog->AddToViewport(200);	

		if(APlayerController* PC = GetOwningPlayer())
		{
			FInputModeUIOnly Mode;
			Mode.SetWidgetToFocus(Dialog->TakeWidget());
			Mode.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);
			PC->SetInputMode(Mode);
			PC->bShowMouseCursor = true;

			AO_LOG(LogJSH, Log, TEXT("HandleJoin: Showing password dialog for Index=%d"), Index);
		}
		else
		{
			AO_LOG(LogJSH, Warning, TEXT("HandleJoin: OwningPlayer is null, cannot set input to dialog"));
		}
	}
	else
	{
		AO_LOG(LogJSH, Error, TEXT("HandleJoin: CreateWidget(PasswordDialog) failed"));
	}
}
