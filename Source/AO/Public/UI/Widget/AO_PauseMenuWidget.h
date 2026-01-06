#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "AO_PauseMenuWidget.generated.h"

class UButton;
class UUserWidget;

UCLASS()
class AO_API UAO_PauseMenuWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	virtual void NativeConstruct() override;
	virtual void NativeOnInitialized() override;
	
	virtual FReply NativeOnPreviewKeyDown(const FGeometry& InGeometry, const FKeyEvent& InKeyEvent) override;

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
	UUserWidget* Btn_Settings = nullptr;

	UPROPERTY(meta=(BindWidget))
	UUserWidget* Btn_ReturnLobby = nullptr;

	UPROPERTY(meta=(BindWidget))
	UUserWidget* Btn_QuitGame = nullptr;

	UPROPERTY(meta=(BindWidget))
	UUserWidget* Btn_Resume = nullptr;

protected:
	UFUNCTION()
	void HandleClicked_Settings();

	UFUNCTION()
	void HandleClicked_ReturnLobby();

	UFUNCTION()
	void HandleClicked_QuitGame();

	UFUNCTION()
	void HandleClicked_Resume();

	UFUNCTION(BlueprintCallable)
	void OnClicked_Settings();

	UFUNCTION(BlueprintCallable)
	void OnClicked_ReturnLobby();

	UFUNCTION(BlueprintCallable)
	void OnClicked_QuitGame();

	UFUNCTION(BlueprintCallable)
	void OnClicked_Resume();
};
