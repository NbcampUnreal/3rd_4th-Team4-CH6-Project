// AO_PlayerController.h (장주만)

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "UI/Widget/AO_UserWidget.h"
#include "AO_PlayerController.generated.h"

struct FPendingTravelInfo {
	FString URL;
	ETravelType Type;
	bool bSeamless;
};

/**
 * 
 */
UCLASS()
class AO_API AAO_PlayerController : public APlayerController
{
	GENERATED_BODY()

public:
	virtual void PreClientTravel(const FString& PendingURL, ETravelType TravelType, bool bIsSeamlessTravel) override;

public:
	void CreateSettingsWidgetInstance(const int32 ZOrder, const ESlateVisibility Visibility);

private:
	void ExecutePreClientTravel();
	
protected:
	UPROPERTY(EditDefaultsOnly, Category = "AO|Widget")
	TSubclassOf<UAO_UserWidget> SettingsWidgetClass;

private:
	UPROPERTY()
	TObjectPtr<UAO_UserWidget> SettingsWidgetInstance = nullptr;

	FPendingTravelInfo PendingTravelInfo;
};
