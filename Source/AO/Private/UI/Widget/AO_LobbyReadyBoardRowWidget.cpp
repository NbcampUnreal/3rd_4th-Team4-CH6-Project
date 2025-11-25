#include "UI/Widget/AO_LobbyReadyBoardRowWidget.h"

#include "Components/TextBlock.h"
#include "UI/Widget/AO_LobbyReadyBoardWidget.h"

void UAO_LobbyReadyBoardRowWidget::Setup(const FAOLobbyReadyBoardEntry& Entry)
{
	if(Txt_PlayerName != nullptr)
	{
		Txt_PlayerName->SetText(FText::FromString(Entry.PlayerName));
	}

	if(Txt_Status != nullptr)
	{
		Txt_Status->SetText(FText::FromString(Entry.StatusLabel));
	}

	// 닉네임 색 (호스트/게스트 구분)
	const FLinearColor HostNameColor(0.2f, 0.8f, 1.0f, 1.0f);   // 하늘색	
	const FLinearColor GuestNameColor(1.0f, 1.0f, 1.0f, 1.0f);  // 흰색

	if(Txt_PlayerName != nullptr)
	{
		const FLinearColor NameColor = Entry.bIsHost ? HostNameColor : GuestNameColor;
		Txt_PlayerName->SetColorAndOpacity(FSlateColor(NameColor));
	}

	// 비활성 상태 색 (진한 회색)
	const FLinearColor InactiveColor(0.2f, 0.2f, 0.2f, 1.0f);
	// 게스트 레디 완료 색 (초록)
	const FLinearColor GuestActiveColor(0.0f, 0.9f, 0.3f, 1.0f);
	// 호스트 “게임 시작 가능” 색 (노랑)
	const FLinearColor HostActiveColor(1.0f, 0.8f, 0.2f, 1.0f);

	if (Txt_Status != nullptr)
	{
		FLinearColor StatusColor = InactiveColor;

		if (Entry.bStatusActive)
		{
			// 호스트/게스트에 따라 다른 색
			StatusColor = Entry.bIsHost ? HostActiveColor : GuestActiveColor;
		}

		Txt_Status->SetColorAndOpacity(FSlateColor(StatusColor));
	}
}
