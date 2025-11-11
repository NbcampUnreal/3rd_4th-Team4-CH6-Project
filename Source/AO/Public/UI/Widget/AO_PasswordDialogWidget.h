// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "AO_PasswordDialogWidget.generated.h"

class UButton;
class UEditableTextBox;
class UAO_LobbyListWidget;

/** 방 참가용 비밀번호 입력 다이얼로그(팝업) */
UCLASS()
class AO_API UAO_PasswordDialogWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	virtual void NativeConstruct() override;

	/** 부모 리스트 설정(뒤로가기/포커스 복구용) */
	UFUNCTION(BlueprintCallable, Category="AO|UI")
	void SetParentList(UAO_LobbyListWidget* InParent)
	{
		ParentList = InParent;
	}

	/** 조인 대상(검색 결과) 인덱스 지정 */
	UFUNCTION(BlueprintCallable, Category="AO|UI")
	void SetTargetIndex(int32 InIndex)
	{
		TargetIndex = InIndex;
	}

protected:
	/** UMG와 동일 이름으로 바인딩 */
	UPROPERTY(meta=(BindWidget)) UEditableTextBox* Txt_Password = nullptr;
	UPROPERTY(meta=(BindWidget)) UButton*        Btn_Ok        = nullptr;
	UPROPERTY(meta=(BindWidget)) UButton*        Btn_Back      = nullptr;

	UFUNCTION() void OnClicked_Ok();
	UFUNCTION() void OnClicked_Back();

private:
	UPROPERTY() TWeakObjectPtr<UAO_LobbyListWidget> ParentList;
	int32 TargetIndex = -1;
};
