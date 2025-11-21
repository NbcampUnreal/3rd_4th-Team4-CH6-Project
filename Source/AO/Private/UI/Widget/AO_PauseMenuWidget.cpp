
#include "UI/Widget/AO_PauseMenuWidget.h"

#include "AO_DelegateManager.h"
#include "AO_Log.h"
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
	// TODO: JM: 위의 델리게이트는 언제 쓰시는지..?
	
	// JM : 설정 위젯 클릭 시 WBP_Settings 열기
	AO_LOG(LogJM, Log, TEXT("Start"));
	if (UAO_DelegateManager* DelegateManager = GetGameInstance()->GetSubsystem<UAO_DelegateManager>())
	{
		DelegateManager->OnSettingsOpen.Broadcast();
		AO_LOG(LogJM, Log, TEXT("Broadcast OnSettingsOpen"));
	}
	else
	{
		AO_LOG(LogJM, Warning, TEXT("Cant Get DelegateManager"));
	}
	AO_LOG(LogJM, Log, TEXT("End"));
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
