//JSH: AO_PlayerController_InGameBase.h

#pragma once

#include "CoreMinimal.h"
#include "AO_PlayerController.h"
#include "GameFramework/PlayerController.h"
#include "Player/PlayerState/AO_PlayerState.h"
#include "UI/Widget/AO_UserWidget.h"
#include "UI/Widget/AO_ConfirmReturnToMenuWidget.h"
#include "AO_PlayerController_InGameBase.generated.h"

class UAO_CameraManagerComponent;
class UAO_PauseMenuWidget;
class UAO_ConfirmReturnToMenuWidget;
class UAO_ConfirmQuitGameWidget;
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
	virtual void AcknowledgePossession(APawn* P) override;
	virtual void BeginPlay() override;
	virtual void SetupInputComponent() override;
	virtual void Tick(float DeltaTime) override;	// JM : 보이스 챗 크래쉬 방지 확인용
	virtual void OnPossess(APawn* InPawn) override;
	virtual void OnRep_Pawn() override;

protected:
	UPROPERTY(Transient)
	TObjectPtr<UAO_PauseMenuWidget> PauseMenuWidgetInstance = nullptr;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<UAO_CameraManagerComponent> CameraManagerComponent;
	
	UPROPERTY(EditDefaultsOnly, Category="AO|UI")
	TSubclassOf<UAO_PauseMenuWidget> PauseMenuClass;

	UPROPERTY()
	UAO_PauseMenuWidget* PauseMenu;
	
	UPROPERTY(EditAnywhere, Category="AO|UI")
	TSubclassOf<UAO_ConfirmReturnToMenuWidget> ConfirmReturnToMenuWidgetClass;

	UPROPERTY()
	UAO_ConfirmReturnToMenuWidget* ConfirmReturnToMenuWidget;
	
	UPROPERTY(EditAnywhere, Category="AO|UI")
	TSubclassOf<UAO_ConfirmQuitGameWidget> ConfirmQuitGameWidgetClass;

	UPROPERTY()
	UAO_ConfirmQuitGameWidget* ConfirmQuitGameWidget;

	UPROPERTY(EditDefaultsOnly, Category="AO|Sound")
	float NotEnoughStaminaSoundInterval = 5.0f;

	double LastNotEnoughStaminaSoundTime = -1e9;

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
	void Client_UpdateVoiceMember(AAO_PlayerState* ChangedPlayerState);

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

	UFUNCTION(Client, Reliable)
	void Client_PrepareForTravel(const FString& URL);

	UFUNCTION(Server, Reliable)
	void Server_NotifyReadyForTravel();

protected:
	void CleanupAudioResource();

protected:
	void HandleUIOpen();
	void ApplyInGameInputDefaults();

	// 위젯 델리게이트 콜백
	UFUNCTION()
	void OnPauseMenu_RequestSettings();

public:		// JM : 실패 화면에서 메인메뉴로 돌아가도록 할 수 있게 해야 하므로 추가
	UFUNCTION(BlueprintCallable, Category="AO")
	void OnPauseMenu_RequestReturnLobby();

protected:
	UFUNCTION()
	void OnPauseMenu_RequestQuitGame();

	UFUNCTION()
	void OnPauseMenu_RequestResume();
	
	UFUNCTION()
	void OpenConfirmReturnToMenuWidget();

	UFUNCTION()
	void OnConfirmReturnToMenu();

	UFUNCTION()
	void OnCancelReturnToMenu();

	UFUNCTION()
	void OnPauseMenu_RequestReturnLobby_OpenConfirm();
	
	UFUNCTION()
	void OpenConfirmQuitGameWidget();

	UFUNCTION()
	void OnConfirmQuitGame();

	UFUNCTION()
	void OnCancelQuitGame();
	
	UFUNCTION()
	void OnPauseMenu_RequestQuitGame_OpenConfirm();

	UAO_OnlineSessionSubsystem* GetOnlineSessionSub() const;

private:
	// 카메라 관리자 초기화
	void InitCameraManager(APawn* InPawn);
	
	/** 모든 보이스 자원이 정리되었는지 확인 */
	bool IsVoiceFullyCleanedUp();

	// ASC Ability 실패 시 바인딩
	void BindASCAbilityFailed();

	// 스태미나 부족으로 Ability 실패할 때 발생하는 함수
	UFUNCTION()
	void HandleAbilityFailed(const UGameplayAbility* Ability, const FGameplayTagContainer& FailureTags);

private:
	bool bIsCheckingVoiceCleanup = false;
};
