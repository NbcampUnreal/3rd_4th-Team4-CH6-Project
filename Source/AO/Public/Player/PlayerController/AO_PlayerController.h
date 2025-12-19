// AO_PlayerController.h (장주만)

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "UI/Widget/AO_UserWidget.h"
#include "AO_PlayerController.generated.h"

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
	TSubclassOf<UAO_UserWidget> GetSettingsWidgetClass() const
	{
		return SettingsWidgetClass;
	}

	UAO_UserWidget* GetOrCreateSettingsWidgetInstance();

public:
	// 삭제: Settings를 미리 AddToViewport 하고 Hidden으로 유지하던 흐름
	// UIStackManager 사용 시에는 필요할 때 Push → Pop으로 관리
	void CreateSettingsWidgetInstance(const int32 ZOrder, const ESlateVisibility Visibility);

private:
	void CleanupAudioResource();
	void UpdateLoadingMapName(const FString& PendingURL) const;


protected:
	UPROPERTY(EditDefaultsOnly, Category = "AO|Widget")
	TSubclassOf<UAO_UserWidget> SettingsWidgetClass;

private:
	UPROPERTY()
	TObjectPtr<UAO_UserWidget> SettingsWidgetInstance = nullptr;
	
};
