// HSJ :  AO_InteractionWidget.h
#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "AO_InteractionWidget.generated.h"

class UAO_InteractionWidgetController;

/**
 * 인터랙션 전용 위젯 (홀딩 프로그레스, 안내 텍스트 표시)
 */
UCLASS(Abstract, BlueprintType)
class AO_API UAO_InteractionWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, Category="Interaction")
	void SetWidgetController(UAO_InteractionWidgetController* InController);

	UFUNCTION(BlueprintImplementableEvent, Category="Interaction")
	void OnWidgetControllerSet();

	UFUNCTION(BlueprintPure, Category="Interaction")
	UAO_InteractionWidgetController* GetWidgetController() const { return WidgetController; }

protected:
	UPROPERTY(BlueprintReadOnly, Category="Interaction")
	TObjectPtr<UAO_InteractionWidgetController> WidgetController;
};