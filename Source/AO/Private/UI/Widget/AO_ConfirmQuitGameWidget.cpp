// AO_ConfirmQuitGameWidget.cpp


#include "UI/Widget/AO_ConfirmQuitGameWidget.h"

#include "Components/Button.h"
#include "Components/TextBlock.h"
#include "UI/AO_UIActionKeySubsystem.h"

void UAO_ConfirmQuitGameWidget::NativeOnInitialized()
{
	Super::NativeOnInitialized();

	/*if (Btn_Confirm != nullptr)
	{
		Btn_Confirm->OnClicked.AddDynamic(this, &UAO_ConfirmQuitGameWidget::HandleClicked_Confirm);
	}

	if (Btn_Cancel != nullptr)
	{
		Btn_Cancel->OnClicked.AddDynamic(this, &UAO_ConfirmQuitGameWidget::HandleClicked_Cancel);
	}

	if (Txt_Message != nullptr)
	{
		Txt_Message->SetText(FText::FromString(TEXT("Are you sure you want to quit the game?")));
	}*/
}

FReply UAO_ConfirmQuitGameWidget::NativeOnPreviewKeyDown(const FGeometry& InGeometry, const FKeyEvent& InKeyEvent)
{
	const FKey Key = InKeyEvent.GetKey();

	APlayerController* PC = GetOwningPlayer();
	if(PC == nullptr)
	{
		return Super::NativeOnPreviewKeyDown(InGeometry, InKeyEvent);
	}

	UGameInstance* GI = PC->GetGameInstance();
	if(GI == nullptr)
	{
		return Super::NativeOnPreviewKeyDown(InGeometry, InKeyEvent);
	}

	UAO_UIActionKeySubsystem* Keys = GI->GetSubsystem<UAO_UIActionKeySubsystem>();
	if(Keys == nullptr)
	{
		return Super::NativeOnPreviewKeyDown(InGeometry, InKeyEvent);
	}

	if(Keys->IsUICloseKey(Key))
	{
		HandleClicked_Cancel();
		return FReply::Handled();
	}

	return Super::NativeOnPreviewKeyDown(InGeometry, InKeyEvent);
}

void UAO_ConfirmQuitGameWidget::HandleClicked_Confirm()
{
	OnConfirmQuitGame.Broadcast();
}

void UAO_ConfirmQuitGameWidget::HandleClicked_Cancel()
{
	OnCancelQuitGame.Broadcast();
}
