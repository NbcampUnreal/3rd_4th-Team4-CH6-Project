// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "AO_LobbyListWidget.generated.h"

class UButton;
class UScrollBox;
class UTextBlock;
class UEditableTextBox;
class UAO_MainMenuWidget;
class UAO_LobbyListEntryWidget;
class UAO_PasswordDialogWidget;
class UAO_OnlineSessionSubsystem;

/**
 * 세션 목록/검색/페이지네이션/비밀번호 진입을 담당하는 로비 리스트 위젯
 */
UCLASS()
class AO_API UAO_LobbyListWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	virtual void NativeConstruct() override;
	virtual void NativeDestruct() override;

	/** 부모 메뉴 설정 (뒤로가기 시 복귀용) */
	UFUNCTION(BlueprintCallable, Category="AO|UI")
	void SetParentMenu(UAO_MainMenuWidget* InParent);

	/** 엔트리에서 호출하는 Join 처리 */
	UFUNCTION(BlueprintCallable, Category="AO|UI")
	void HandleJoin(int32 Index, bool bNeedsPassword);

protected:
	/* ---------- 기본 UI ---------- */
	UPROPERTY(meta=(BindWidget)) UButton* Btn_Refresh = nullptr;
	UPROPERTY(meta=(BindWidget)) UButton* Btn_Close   = nullptr;
	UPROPERTY(meta=(BindWidget)) UScrollBox* Scroll_SessionList = nullptr;

	UPROPERTY(EditDefaultsOnly, Category="AO|UI")
	TSubclassOf<UAO_LobbyListEntryWidget> LobbyEntryClass;

	UPROPERTY(EditDefaultsOnly, Category="AO|UI")
	TSubclassOf<UAO_PasswordDialogWidget> PasswordDialogClass;

	/** 부모 메뉴 폴백 생성용 */
	UPROPERTY(EditDefaultsOnly, Category="AO|UI")
	TSubclassOf<UAO_MainMenuWidget> MainMenuClass;

	UFUNCTION() void OnClicked_Refresh();
	UFUNCTION() void OnClicked_Close();
	UFUNCTION() void OnFindSessionsComplete(bool bSuccessful);

	/* ---------- 검색/페이지 ---------- */
	UPROPERTY(meta=(BindWidgetOptional)) UEditableTextBox* Edt_Search = nullptr;
	UPROPERTY(meta=(BindWidgetOptional)) UButton* Btn_Prev = nullptr;
	UPROPERTY(meta=(BindWidgetOptional)) UButton* Btn_Next = nullptr;
	UPROPERTY(meta=(BindWidgetOptional)) UTextBlock* Txt_PageInfo = nullptr;

	UFUNCTION() void OnSearchTextCommitted(const FText& Text, ETextCommit::Type CommitMethod);
	UFUNCTION() void OnClicked_PrevPage();
	UFUNCTION() void OnClicked_NextPage();

	void RebuildList();

private:
	/** 부모 메뉴 위젯 (입력 복귀용) */
	UPROPERTY()
	TObjectPtr<UAO_MainMenuWidget> ParentMenu = nullptr;

	/** 세션 서브시스템 접근 */
	UAO_OnlineSessionSubsystem* GetSub() const;

	/* ---------- 상태 ---------- */
	static constexpr int32 kItemsPerPage = 5;

	UPROPERTY() FString CurrentSearch = TEXT("");
	UPROPERTY() int32 PageIndex = 0;
	UPROPERTY() TArray<int32> FilteredIndices;

	void RebuildFilter();
	void ClampAndUpdatePage();
	void UpdatePageUI();
	int32 GetTotalPages() const;
};
