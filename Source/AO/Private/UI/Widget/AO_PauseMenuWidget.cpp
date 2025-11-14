
#include "UI/Widget/AO_PauseMenuWidget.h"
#include "Components/Button.h"

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
