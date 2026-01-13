// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "AO_MainMenuWidget.generated.h"

class UButton;

class UAO_LobbyListWidget;
class UAO_HostDialogWidget;

/** AO 메인 메뉴: Host / Join / Settings / Quit */
UCLASS()
class AO_API UAO_MainMenuWidget : public UUserWidget
{
	GENERATED_BODY()

protected:
	/* === Class Slots (에디터에서 지정) === */
	UPROPERTY(EditDefaultsOnly, Category="AO|UI")
	TSubclassOf<UAO_HostDialogWidget> HostDialogClass;
	
	UPROPERTY(EditDefaultsOnly, Category="AO|UI")
	TSubclassOf<UAO_LobbyListWidget> LobbyListWidgetClass;

	/* === Click Handlers === */
	UFUNCTION(BlueprintCallable, Category="AO|UI")
	void OnClicked_Host();
	
	UFUNCTION(BlueprintCallable, Category="AO|UI")
	void OnClicked_Join();
	
	UFUNCTION(BlueprintCallable, Category="AO|UI")
	void OnClicked_Settings();
	
	UFUNCTION(BlueprintCallable, Category="AO|UI")
	void OnClicked_Quit();
};
