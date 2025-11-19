#include "UI/Widget/AO_LobbyReadyBoardWidget.h"

#include "Components/VerticalBox.h"
#include "Components/VerticalBoxSlot.h"
#include "UI/Widget/AO_LobbyReadyBoardRowWidget.h"
#include "AO_Log.h"


void UAO_LobbyReadyBoardWidget::SetEntries(const TArray<FAOLobbyReadyBoardEntry>& InEntries)
{
	AO_LOG(LogJSH, Log, TEXT("LobbyBoardWidget::SetEntries  Num=%d  RowClass=%s  VB=%s"),
		InEntries.Num(),
		*GetNameSafe(RowWidgetClass),
		*GetNameSafe(VB_Entries));

	if(VB_Entries == nullptr)
	{
		AO_LOG(LogJSH, Error, TEXT(" → Abort: VB_Entries is null"));
		return;
	}

	VB_Entries->ClearChildren();

	if(RowWidgetClass == nullptr)
	{
		AO_LOG(LogJSH, Error, TEXT(" → Abort: RowWidgetClass is null"));
		return;
	}

	UWorld* World = GetWorld();
	if(World == nullptr)
	{
		AO_LOG(LogJSH, Error, TEXT(" → Abort: World is null"));
		return;
	}

	for(const FAOLobbyReadyBoardEntry& Entry : InEntries)
	{
		AO_LOG(LogJSH, Log, TEXT(" → Create Row for: %s / %s / Active=%d"),
			*Entry.PlayerName,
			*Entry.StatusLabel,
			(int)Entry.bStatusActive);

		UAO_LobbyReadyBoardRowWidget* RowWidget =
			CreateWidget<UAO_LobbyReadyBoardRowWidget>(World, RowWidgetClass);

		if(RowWidget == nullptr)
		{
			AO_LOG(LogJSH, Error, TEXT(" → CreateWidget FAILED"));
			continue;
		}

		RowWidget->Setup(Entry);

		if(UVerticalBoxSlot* EntrySlot = Cast<UVerticalBoxSlot>(VB_Entries->AddChild(RowWidget)))
		{
			AO_LOG(LogJSH, Log, TEXT(" → Added Row to VerticalBox"));
			EntrySlot->SetPadding(FMargin(0.0f, 2.0f));
		}
		else
		{
			AO_LOG(LogJSH, Error, TEXT(" → AddChild returned NON-VerticalBoxSlot"));
		}
	}
}
