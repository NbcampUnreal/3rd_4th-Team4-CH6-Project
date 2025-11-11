// Fill out your copyright...

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "AO_MainMenuPlayerController.generated.h"

class UAO_MainMenuWidget;

UCLASS()
class AO_API AAO_MainMenuPlayerController : public APlayerController
{
	GENERATED_BODY()

public:
	virtual void BeginPlay() override;

protected:
	/** 화면에 띄울 메인 메뉴 위젯 클래스 (BP_MainMenu 등) */
	UPROPERTY(EditDefaultsOnly, Category="AO|UI")
	TSubclassOf<UAO_MainMenuWidget> MainMenuClass;

private:
	UPROPERTY() UAO_MainMenuWidget* MainMenu = nullptr;
};
