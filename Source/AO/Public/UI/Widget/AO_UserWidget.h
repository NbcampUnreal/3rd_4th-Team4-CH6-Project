// AO_UserWidget.h (장주만)

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "AO_UserWidget.generated.h"

/**
 * 1. 향후 전체 위젯에 공통적으로 적용해야 할 기능이 생겼을 때, 바로 적용하기 위함
 * (확장성을 고려한 설계)
 * 2. WBP_Base_{category} 를 통해 최상위 부모의 기능을 하위 WBP에서 모두 공유하여 사용
 *  - WBP_Base_Modal : 팝업/모달 류 WBP는 해당 클래스를 상속받아서 일괄적으로 Open/Close 애니메이션이 적용되도록 함
 */
UCLASS()
class AO_API UAO_UserWidget : public UUserWidget
{
	GENERATED_BODY()

public:

protected:
	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, Category = "Widget Config")
	float HoverOpacity = 1.0f;
	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, Category = "Widget Config")
	float UnHoverOpacity = 0.5f;
};
