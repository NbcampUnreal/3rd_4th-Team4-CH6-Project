
#include "UI/Widget/AO_PauseMenuWidget.h"

#include "AO_DelegateManager.h"
#include "AO_Log.h"
#include "Components/Button.h"
#include "UI/AO_UIActionKeySubsystem.h"

void UAO_PauseMenuWidget::NativeConstruct()
{
	Super::NativeConstruct();
}

void UAO_PauseMenuWidget::NativeOnInitialized()
{
	Super::NativeOnInitialized();

	if (Btn_Settings)
	{
		Btn_Settings->OnClicked.AddDynamic(this, &ThisClass::HandleClicked_Settings);
	}

	if (Btn_ReturnLobby)
	{
		Btn_ReturnLobby->OnClicked.AddDynamic(this, &ThisClass::HandleClicked_ReturnLobby);
	}

	if (Btn_QuitGame)
	{
		Btn_QuitGame->OnClicked.AddDynamic(this, &ThisClass::HandleClicked_QuitGame);
	}

	if (Btn_Resume)
	{
		Btn_Resume->OnClicked.AddDynamic(this, &ThisClass::HandleClicked_Resume);
	}
}

FReply UAO_PauseMenuWidget::NativeOnPreviewKeyDown(const FGeometry& InGeometry, const FKeyEvent& InKeyEvent)
{
	const FKey Key = InKeyEvent.GetKey();

	if (APlayerController* PC = GetOwningPlayer())
	{
		if (UGameInstance* GI = PC->GetGameInstance())
		{
			if (UAO_UIActionKeySubsystem* Keys = GI->GetSubsystem<UAO_UIActionKeySubsystem>())
			{
				if (Keys->IsUICloseKey(Key))
				{
					OnRequestResume.Broadcast();
					return FReply::Handled();
				}
			}
		}
	}

	return Super::NativeOnPreviewKeyDown(InGeometry, InKeyEvent);
}

void UAO_PauseMenuWidget::HandleClicked_Settings()
{
	OnRequestSettings.Broadcast();
}

void UAO_PauseMenuWidget::HandleClicked_ReturnLobby()
{
	OnRequestReturnLobby.Broadcast();
}

void UAO_PauseMenuWidget::HandleClicked_QuitGame()
{
	OnRequestQuitGame.Broadcast();
}

void UAO_PauseMenuWidget::HandleClicked_Resume()
{
	OnRequestResume.Broadcast();
}
