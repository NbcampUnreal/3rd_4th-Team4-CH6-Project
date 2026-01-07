// AO_ConfirmQuitGameWidget.h

#pragma once

#include "CoreMinimal.h"
#include "UI/Widget/AO_UserWidget.h"
#include "AO_ConfirmQuitGameWidget.generated.h"

class UButton;
class UTextBlock;

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnConfirmQuitGame);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnCancelQuitGame);

/**
 *
 */
UCLASS()
class AO_API UAO_ConfirmQuitGameWidget : public UAO_UserWidget
{
	GENERATED_BODY()

public:
	virtual void NativeOnInitialized() override;
	virtual FReply NativeOnPreviewKeyDown(const FGeometry& InGeometry, const FKeyEvent& InKeyEvent) override;

public:
	UPROPERTY(BlueprintAssignable, Category = "Confirm")
	FOnConfirmQuitGame OnConfirmQuitGame;

	UPROPERTY(BlueprintAssignable, Category = "Confirm")
	FOnCancelQuitGame OnCancelQuitGame;

protected:
	/* WBP Application Btn으로 수정 (JM)
	 *UPROPERTY(meta = (BindWidget))
	UButton* Btn_Confirm;

	UPROPERTY(meta = (BindWidget))
	UButton* Btn_Cancel;

	UPROPERTY(meta = (BindWidget, OptionalWidget = true))
	UTextBlock* Txt_Message;*/

protected:
	UFUNCTION(BlueprintCallable)
	void HandleClicked_Confirm();

	UFUNCTION(BlueprintCallable)
	void HandleClicked_Cancel();
};