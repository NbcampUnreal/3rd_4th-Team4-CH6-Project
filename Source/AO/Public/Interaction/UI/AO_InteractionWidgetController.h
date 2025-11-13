// HSJ :  AO_InteractionWidgetController.h
#pragma once

#include "CoreMinimal.h"
#include "UObject/Object.h"
#include "Interaction/Interface/AO_InteractionInfo.h"
#include "AO_InteractionWidgetController.generated.h"

UENUM(BlueprintType)
enum class EAO_InteractionMessageType : uint8
{
	Notice,
	Progress
};

USTRUCT(BlueprintType)
struct FAO_InteractionMessage
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadWrite)
	EAO_InteractionMessageType MessageType = EAO_InteractionMessageType::Notice;

	UPROPERTY(BlueprintReadWrite)
	TObjectPtr<AActor> Instigator = nullptr;

	UPROPERTY(BlueprintReadWrite)
	bool bShouldRefresh = false;

	UPROPERTY(BlueprintReadWrite)
	bool bSwitchActive = false;

	UPROPERTY(BlueprintReadWrite)
	FAO_InteractionInfo InteractionInfo;
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnInteractionMessageReceived, FAO_InteractionMessage, Message);

/**
 * 인터랙션 위젯을 위한 컨트롤러
 * WBP에서 델리게이트를 바인딩하여 UI 업데이트
 */
UCLASS(BlueprintType, Blueprintable)
class AO_API UAO_InteractionWidgetController : public UObject
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, Category="Interaction")
	void BroadcastInteractionMessage(FAO_InteractionMessage Message);

	UFUNCTION(BlueprintCallable, Category="Interaction")
	void HideInteraction();

	UPROPERTY(BlueprintAssignable, Category="Interaction")
	FOnInteractionMessageReceived OnInteractionMessageReceived;
};