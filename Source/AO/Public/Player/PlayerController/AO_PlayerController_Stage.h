// AO_PlayerController_Stage.h (장주만)

#pragma once

#include "CoreMinimal.h"
#include "AO_PlayerController_InGameBase.h"
#include "AO_PlayerController_Stage.generated.h"

/**
 * 
 */
UCLASS()
class AO_API AAO_PlayerController_Stage : public AAO_PlayerController_InGameBase
{
	GENERATED_BODY()

public:
	AAO_PlayerController_Stage();


protected:
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;	// JM : 생명주기 테스트용

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "AO|UI")
	TSubclassOf<UUserWidget> HUDWidgetClass;

	UPROPERTY()
	TObjectPtr<UUserWidget> HUDWidget;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "AO|UI")
	TSubclassOf<UUserWidget> DeathWidgetClass;

	UPROPERTY()
	TObjectPtr<UUserWidget> DeathWidget;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "AO|UI")
	TSubclassOf<UUserWidget> SpectateWidgetClass;

	UPROPERTY()
	TObjectPtr<UUserWidget> SpectateWidget;

public:
	// 서버로 출발 보내는 RPC
	UFUNCTION(Server, Reliable)
    void Server_RequestStageExit();

	UFUNCTION(BlueprintCallable, Category = "AO|UI")
	void ShowDeathUI();

	UFUNCTION(BlueprintCallable, Category = "AO|UI")
	void RequestSpectate();
	
	UFUNCTION(BlueprintCallable, Category = "AO|UI")
	void RequestSpectateNext(bool bForward);

protected:
	UFUNCTION(Server, Reliable)
	void ServerRPC_RequestSpectate();
	
	UFUNCTION(Server, Reliable)
	void ServerRPC_RequestSpectateNext(bool bForward);

	UFUNCTION(Client, Reliable)
	void ClientRPC_SetSpectateTarget(APawn* NewTarget, int32 NewPlayerIndex);

	UPROPERTY(BlueprintReadWrite)
	TObjectPtr<APawn> CurrentSpectateTarget;

	int32 CurrentSpectatePlayerIndex = INDEX_NONE;

	TObjectPtr<APawn> FindNextSpectateTarget(bool bForward, int32& OutNewIndex);
	
	/* ----------테스트용 임시 코드------------*/
public:
	// 서버로 실패 요청 보내는 RPC
	UFUNCTION(Server, Reliable)
	void Server_RequestStageFail();
	
protected:
	void SetupInputComponent() override;
	
	// O 키 입력 처리 (클라이언트)
	void HandleStageFailInput();
	/* --------------------------------------*/
};
