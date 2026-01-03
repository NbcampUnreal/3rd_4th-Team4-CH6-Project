// AO_NameTagWidget.h

#pragma once

#include "CoreMinimal.h"
#include "UI/Widget/AO_UserWidget.h"
#include "AO_NameTagWidget.generated.h"

class UImage;
class UTextBlock;

UCLASS()
class AO_API UAO_NameTagWidget : public UAO_UserWidget
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable)
	void SetPlayerName(const FText& InName);

	UFUNCTION(BlueprintCallable)
	void SetUIScale(float InScale);

	UFUNCTION(BlueprintCallable)
	void SetPlayerTalkingVisibility(const bool bIsTalking);
	
protected:
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> Text_Name;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UImage> Img_IsTalking;
};
