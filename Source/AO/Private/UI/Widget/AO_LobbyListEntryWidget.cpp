#include "UI/Widget/AO_LobbyListEntryWidget.h"
#include "UI/Widget/AO_LobbyListWidget.h"
#include "Components/Button.h"
#include "Components/TextBlock.h"
#include "Components/Image.h"

void UAO_LobbyListEntryWidget::NativeConstruct()
{
	Super::NativeConstruct();

	if(Btn_Join && !Btn_Join->OnClicked.IsAlreadyBound(this, &ThisClass::OnClicked_Join))
	{
		Btn_Join->OnClicked.AddDynamic(this, &ThisClass::OnClicked_Join);
	}
}

void UAO_LobbyListEntryWidget::Setup(int32 InIndex, const FString& InTitle, int32 InOpenSlots, int32 InMaxSlots, bool bInNeedsPassword)
{
	Index = InIndex;
	bNeedsPassword = bInNeedsPassword;
	OpenSlots = InOpenSlots;
	MaxSlots = InMaxSlots;

	if(Txt_Title)
	{
		Txt_Title->SetText(FText::FromString(InTitle));
	}

	if(Txt_Slots)
	{
		const int32 CurPlayers = FMath::Clamp(MaxSlots - OpenSlots, 0, MaxSlots);
		Txt_Slots->SetText(FText::FromString(FString::Printf(TEXT("%d/%d"), CurPlayers, MaxSlots)));
	}

	if(Img_Lock)
	{
		Img_Lock->SetVisibility(bNeedsPassword ? ESlateVisibility::Visible : ESlateVisibility::Collapsed);
	}
}

void UAO_LobbyListEntryWidget::OnClicked_Join()
{
	if(ParentLobby.IsValid())
	{
		ParentLobby->HandleJoin(Index, bNeedsPassword);
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("AO_LobbyListEntry: ParentLobby is invalid."));
	}
}

UAO_LobbyListWidget* UAO_LobbyListEntryWidget::GetParentLobby() const
{
	return GetTypedOuter<UAO_LobbyListWidget>();
}
