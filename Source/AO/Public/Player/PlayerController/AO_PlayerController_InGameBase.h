// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "AO_PlayerController_InGameBase.generated.h"

class UAO_PauseMenuWidget;
class UAO_OnlineSessionSubsystem;

/**
 * 
 */
UCLASS()
class AO_API AAO_PlayerController_InGameBase : public APlayerController
{
	GENERATED_BODY()

public:
	AAO_PlayerController_InGameBase();

protected:
	virtual void BeginPlay() override;
	virtual void SetupInputComponent() override;

protected:
	UPROPERTY(EditDefaultsOnly, Category="AO|UI")
	TSubclassOf<UAO_PauseMenuWidget> PauseMenuClass;

	UPROPERTY()
	UAO_PauseMenuWidget* PauseMenu;

	UPROPERTY()
	bool bPauseMenuVisible;

protected:
	void TogglePauseMenu();

	// 위젯 델리게이트 콜백
	UFUNCTION()
	void OnPauseMenu_RequestSettings();

	UFUNCTION()
	void OnPauseMenu_RequestReturnLobby();

	UFUNCTION()
	void OnPauseMenu_RequestQuitGame();

	UFUNCTION()
	void OnPauseMenu_RequestResume();

	// 내부 헬퍼
	void ShowPauseMenu();
	void HidePauseMenu();

	UAO_OnlineSessionSubsystem* GetOnlineSessionSub() const;
};
