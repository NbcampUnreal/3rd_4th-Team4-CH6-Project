#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "AO_HostDialogWidget.generated.h"

class UButton;
class UEditableTextBox;

/** 방 이름/비밀번호를 입력받아 HostSessionEx를 호출하는 마우스 중심 팝업 다이얼로그 */
UCLASS()
class AO_API UAO_HostDialogWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	virtual void NativeConstruct() override;

protected:
	UPROPERTY(meta=(BindWidget)) UEditableTextBox* Txt_RoomName = nullptr;
	UPROPERTY(meta=(BindWidget)) UEditableTextBox* Txt_Password = nullptr;
	UPROPERTY(meta=(BindWidget)) UButton* Btn_Create = nullptr;
	UPROPERTY(meta=(BindWidget)) UButton* Btn_Cancel = nullptr;
	UPROPERTY(meta=(BindWidget)) UButton* Btn_Backdrop = nullptr;

	UFUNCTION() void OnClicked_Create();
	UFUNCTION() void OnClicked_Cancel();
	UFUNCTION() void OnClicked_Backdrop();

	/* 입력 변화 콜백 (실시간 버튼 상태 갱신) */
	UFUNCTION() void OnRoomNameChanged(const FText& Text);
	UFUNCTION() void OnRoomNameCommitted(const FText& Text, ETextCommit::Type Method);

	/* 모달 입력 적용 (커서만 표시, 포커스: 방 이름 상자) */
	void ApplyModalInput();

	/* 버튼 활성화/비활성 갱신 */
	void UpdateCreateButtonState();

	/* 유효성 검사/정규화 */
	bool IsValidRoomName(const FString& Name) const;
	FString GetTrimmedRoomName() const;
	FString GetTrimmedPassword() const;
};
