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

	/* 모달 입력 적용 (커서만 표시, 포커스 없음) */
	void ApplyModalInput();
};
