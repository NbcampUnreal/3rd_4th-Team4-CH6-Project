// AO_PlayerController.h (장주만)

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "UI/Widget/AO_UserWidget.h"
#include "AO_PlayerController.generated.h"

// JM : ClientTravel 에 딜레이를 주기 위함
// 딜레이 제대로 동작하지 않아서 삭제 예정
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

// protected:
	// virtual void ClientTravelInternal_Implementation(const FString& URL, enum ETravelType TravelType, bool bSeamless = false, FGuid MapPackageGuid = FGuid()) override;

public:
	void CreateSettingsWidgetInstance(const int32 ZOrder, const ESlateVisibility Visibility);

private:
	// void ExecutePreClientTravel();
	// void ExecuteClientTravelInternal(FString URL, ETravelType TravelType, bool bSeamless, FGuid MapPackageGuid);
	
protected:
	UPROPERTY(EditDefaultsOnly, Category = "AO|Widget")
	TSubclassOf<UAO_UserWidget> SettingsWidgetClass;

private:
	UPROPERTY()
	TObjectPtr<UAO_UserWidget> SettingsWidgetInstance = nullptr;

	// FPendingTravelInfo PendingTravelInfo;
	// FTimerHandle DelayedTravelTimerHandle;
};
