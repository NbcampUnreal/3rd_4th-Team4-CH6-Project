// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AO_PlayerController_InGameBase.h"
#include "AO_PlayerController_Lobby.generated.h"

class ATargetPoint;
class AAO_CustomizingCharacter;
/**
 * 
 */
UCLASS()
class AO_API AAO_PlayerController_Lobby : public AAO_PlayerController_InGameBase
{
	GENERATED_BODY()

public:
	AAO_PlayerController_Lobby();

protected:
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;	// JM : 생명주기 테스트용

public:
	/* 레디 상태를 서버에 전달 */
	UFUNCTION(Server, Reliable)
	void Server_SetReady(bool bNewReady);

	/* 시작 요청을 서버에 전달 */
	UFUNCTION(Server, Reliable)
	void Server_RequestStart();

	/* 초대 UI 요청을 서버로 전달 */
	UFUNCTION(Server, Reliable)
	void Server_RequestInviteOverlay();

	/* 옷장 UI 요청을 서버로 전달 */
	UFUNCTION(Server, Reliable)
	void Server_RequestWardrobe();

	/* 서버에서 해당 클라에게 초대 UI 열라고 지시 */
	UFUNCTION(Client, Reliable)
	void Client_OpenInviteOverlay();

	/* 서버에서 해당 클라에게 옷장 UI 열라고 지시 */
	UFUNCTION(Client, Reliable)
	void Client_OpenWardrobe();

	/* 초대 UI 여는 로컬 함수 */
	UFUNCTION()
	void OpenInviteOverlay();

	/* 옷장 UI 여는 로컬 함수 */
	UFUNCTION()
	void OpenWardrobe();

	UFUNCTION(BlueprintCallable, Category = "Customizing")
	void CloseWardrobe();

	void FadeIn();
	void FadeOut();

	void OnFadeInFinishedOpenUI();
	void OnFadeInFinishedCloseUI();

protected:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Customizing")
	TObjectPtr<UAO_UserWidget> CustomizingWidget = nullptr;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Customizing")
	float FadeTime = 1.5f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Customizing")
	TSubclassOf<AAO_CustomizingCharacter> CustomizingDummyClass;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Customizing")
	TObjectPtr<AAO_CustomizingCharacter> CustomizingDummy = nullptr;

	UPROPERTY()
	TObjectPtr<AAO_PlayerCharacter> PlayerCharacter = nullptr;
	
	FTimerHandle FadeTimerHandle;
};
