#include "UI/Widget/AO_LobbyListEntryWidget.h"
#include "UI/Widget/AO_LobbyListWidget.h"
#include "Components/Button.h"
#include "Components/TextBlock.h"
#include "Components/Image.h"
#include "AO/AO_Log.h"

void UAO_LobbyListEntryWidget::NativeConstruct()
{
	Super::NativeConstruct();

	if (Btn_Join && !Btn_Join->OnClicked.IsAlreadyBound(this, &ThisClass::OnClicked_Join))
	{
		Btn_Join->OnClicked.AddDynamic(this, &ThisClass::OnClicked_Join);
	}
	else if (!Btn_Join)
	{
		AO_LOG(LogJSH, Warning, TEXT("NativeConstruct: Btn_Join is null"));
	}
}

void UAO_LobbyListEntryWidget::Setup(int32 InIndex, const FString& InTitle, int32 InOpenSlots, int32 InMaxSlots, bool bInNeedsPassword)
{
	Index = InIndex;
	bNeedsPassword = bInNeedsPassword;
	OpenSlots = InOpenSlots;
	MaxSlots = InMaxSlots;

	AO_LOG(LogJSH, Log, TEXT("Setup: Index=%d, Title=%s, OpenSlots=%d, MaxSlots=%d, NeedsPw=%d"),
		Index, *InTitle, InOpenSlots, InMaxSlots, static_cast<int32>(bInNeedsPassword));

	if (Txt_Title)
	{
		Txt_Title->SetText(FText::FromString(InTitle));
	}
	else
	{
		AO_LOG(LogJSH, Warning, TEXT("Setup: Txt_Title is null"));
	}

	if (Txt_Slots)
	{
		const int32 CurPlayers = FMath::Clamp(MaxSlots - OpenSlots, 0, MaxSlots);
		Txt_Slots->SetText(FText::FromString(FString::Printf(TEXT("%d/%d"), CurPlayers, MaxSlots)));
	}
	else
	{
		AO_LOG(LogJSH, Warning, TEXT("Setup: Txt_Slots is null"));
	}

	if (Img_Lock)
	{
		Img_Lock->SetVisibility(bNeedsPassword ? ESlateVisibility::Visible : ESlateVisibility::Collapsed);
	}
	else
	{
		AO_LOG(LogJSH, Warning, TEXT("Setup: Img_Lock is null"));
	}
}

void UAO_LobbyListEntryWidget::OnClicked_Join()
{
	AO_LOG(LogJSH, Log, TEXT("OnClicked_Join: Index=%d, NeedsPw=%d"),
		Index, static_cast<int32>(bNeedsPassword));

	if (ParentLobby.IsValid())
	{
		ParentLobby->HandleJoin(Index, bNeedsPassword);
	}
	else
	{
		AO_LOG(LogJSH, Warning, TEXT("OnClicked_Join: ParentLobby is invalid"));
	}
}

UAO_LobbyListWidget* UAO_LobbyListEntryWidget::GetParentLobby() const
{
	return GetTypedOuter<UAO_LobbyListWidget>();
}
