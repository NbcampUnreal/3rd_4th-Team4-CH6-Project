// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AO_PlayerController.h"
#include "GameFramework/PlayerController.h"
#include "Player/PlayerState/AO_PlayerState.h"
#include "UI/Widget/AO_UserWidget.h"
#include "AO_PlayerController_InGameBase.generated.h"

class UAO_CameraManagerComponent;
class UAO_PauseMenuWidget;
class UAO_OnlineSessionSubsystem;

/**
 *
 */
UCLASS()
class AO_API AAO_PlayerController_InGameBase : public AAO_PlayerController
{
	GENERATED_BODY()

public:
	AAO_PlayerController_InGameBase();

protected:
	virtual void OnPossess(APawn* InPawn) override;
	virtual void OnRep_Pawn() override;
	virtual void BeginPlay() override;
	virtual void SetupInputComponent() override;

protected:
	UPROPERTY(Transient)
	TObjectPtr<UAO_PauseMenuWidget> PauseMenuWidgetInstance = nullptr;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<UAO_CameraManagerComponent> CameraManagerComponent;
	
	UPROPERTY(EditDefaultsOnly, Category="AO|UI")
	TSubclassOf<UAO_PauseMenuWidget> PauseMenuClass;

	UPROPERTY()
	UAO_PauseMenuWidget* PauseMenu;

public:
	TSubclassOf<UAO_PauseMenuWidget> GetPauseMenuWidgetClass() const { return PauseMenuClass; }
	
	UAO_PauseMenuWidget* GetOrCreatePauseMenuWidget();

// JM : Voice Chat
public:
	UFUNCTION(Client, Reliable)
	void Client_StartVoiceChat();

	UFUNCTION(Client, Reliable)
	void Client_StopVoiceChat();

	UFUNCTION(Client, Reliable)
	void Client_UpdateVoiceMember(AAO_PlayerState* DeadPlayerState);

	UFUNCTION(Client, Reliable)
	void Client_UnmuteVoiceMember(AAO_PlayerState* AlivePlayerState);

	UFUNCTION(Server, Reliable)
	void Test_Server_SelfDie();

	UFUNCTION(Server, Reliable)
	void Test_Server_SelfAlive();

	UFUNCTION(BlueprintCallable, Category="AO|Test")
	void Test_Die();

	UFUNCTION(BlueprintCallable, Category="AO|Test")
	void Test_Alive();

protected:
	void HandleUIOpen();

	// 위젯 델리게이트 콜백
	UFUNCTION()
	void OnPauseMenu_RequestSettings();

	UFUNCTION()
	void OnPauseMenu_RequestReturnLobby();

	UFUNCTION()
	void OnPauseMenu_RequestQuitGame();

	UFUNCTION()
	void OnPauseMenu_RequestResume();

	UAO_OnlineSessionSubsystem* GetOnlineSessionSub() const;

private:
	// 카메라 관리자 초기화
	void InitCameraManager();
};
