// Fill out your copyright...

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "UI/Widget/AO_UserWidget.h"
#include "AO_PlayerController_MainMenu.generated.h"

class UAO_MainMenuWidget;

UCLASS()
class AO_API AAO_PlayerController_MainMenu : public APlayerController
{
	GENERATED_BODY()

public:
	virtual void BeginPlay() override;

protected:
	/** 화면에 띄울 메인 메뉴 위젯 클래스 (BP_MainMenu 등) */
	UPROPERTY(EditDefaultsOnly, Category="AO|UI")
	TSubclassOf<UAO_MainMenuWidget> MainMenuClass;

	UPROPERTY(EditDefaultsOnly, Category="AO|UI")
	TSubclassOf<UAO_UserWidget> SettingClass;		// JM 코드추가 : 세팅 위젯

private:
	UPROPERTY() UAO_MainMenuWidget* MainMenu = nullptr;

	UPROPERTY()
	UAO_UserWidget* Settings = nullptr;				// JM 코드추가 : 세팅 위젯
};
