#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "AO_PauseMenuWidget.generated.h"

class UButton;

UCLASS()
class AO_API UAO_PauseMenuWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	virtual void NativeConstruct() override;
	void NativeOnInitialized();

public:
	DECLARE_DYNAMIC_MULTICAST_DELEGATE(FAO_OnPauseMenuRequestSettings);
	DECLARE_DYNAMIC_MULTICAST_DELEGATE(FAO_OnPauseMenuRequestReturnLobby);
	DECLARE_DYNAMIC_MULTICAST_DELEGATE(FAO_OnPauseMenuRequestQuitGame);
	DECLARE_DYNAMIC_MULTICAST_DELEGATE(FAO_OnPauseMenuRequestResume);

public:
	UPROPERTY(BlueprintAssignable, Category="AO|PauseMenu")
	FAO_OnPauseMenuRequestSettings OnRequestSettings;

	UPROPERTY(BlueprintAssignable, Category="AO|PauseMenu")
	FAO_OnPauseMenuRequestReturnLobby OnRequestReturnLobby;

	UPROPERTY(BlueprintAssignable, Category="AO|PauseMenu")
	FAO_OnPauseMenuRequestQuitGame OnRequestQuitGame;

	UPROPERTY(BlueprintAssignable, Category="AO|PauseMenu")
	FAO_OnPauseMenuRequestResume OnRequestResume;

protected:
	UPROPERTY(meta=(BindWidget))
	UButton* Btn_Settings;

	UPROPERTY(meta=(BindWidget))
	UButton* Btn_ReturnLobby;

	UPROPERTY(meta=(BindWidget))
	UButton* Btn_QuitGame;

	UPROPERTY(meta=(BindWidget))
	UButton* Btn_Resume;

protected:
	UFUNCTION()
	void HandleClicked_Settings();

	UFUNCTION()
	void HandleClicked_ReturnLobby();

	UFUNCTION()
	void HandleClicked_QuitGame();

	UFUNCTION()
	void HandleClicked_Resume();
};
