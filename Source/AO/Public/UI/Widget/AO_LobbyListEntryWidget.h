// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "AO/Public/UI/Widget/AO_LobbyListWidget.h"
#include "AO_LobbyListEntryWidget.generated.h"

class UButton;
class UTextBlock;
class UImage;



/** 세션 리스트 한 줄 항목 */
UCLASS()
class AO_API UAO_LobbyListEntryWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	virtual void NativeConstruct() override;

	/** Index / 방제목 / 남은슬롯 / 최대슬롯 / 비번여부 세팅 */
	UFUNCTION(BlueprintCallable, Category="AO|UI")
	void Setup(int32 InIndex, const FString& InTitle, int32 InOpenSlots, int32 InMaxSlots, bool bInNeedsPassword);

	UFUNCTION(BlueprintCallable, Category="AO|UI")
	void SetParentLobby(UAO_LobbyListWidget* InParent) { ParentLobby = InParent; }

protected:
	UPROPERTY(meta=(BindWidget)) UButton* Btn_Join = nullptr;
	UPROPERTY(meta=(BindWidget)) UTextBlock* Txt_Title = nullptr;
	UPROPERTY(meta=(BindWidget)) UTextBlock* Txt_Slots = nullptr;
	UPROPERTY(meta=(BindWidgetOptional)) UImage* Img_Lock = nullptr;

	UFUNCTION() void OnClicked_Join();

private:
	UPROPERTY() TWeakObjectPtr<UAO_LobbyListWidget> ParentLobby;

	int32 Index = -1;
	bool  bNeedsPassword = false;
	int32 MaxSlots = 0;
	int32 OpenSlots = 0;

	UAO_LobbyListWidget* GetParentLobby() const;
};
