// AO_DelegateManager.h (장주만)

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "AO_DelegateManager.generated.h"

// 델리게이트 정의 :
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnSettingsOpenDelegate);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnSettingsCloseDelegate);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnResetAllSettings);


/* 전역 델리게이트 매니저*/
UCLASS()
class AO_API UAO_DelegateManager : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:	// 델리게이트 정의
	UPROPERTY(BlueprintAssignable, Category = "Delegate")
	FOnSettingsOpenDelegate OnSettingsOpen;

	UPROPERTY(BlueprintAssignable, Category = "Delegate")
	FOnSettingsOpenDelegate OnSettingsClose;

	UPROPERTY(BlueprintAssignable, Category = "Delegate")
	FOnResetAllSettings OnResetAllSettings;


public:	// 델리게이트 호출 (BP에서 이벤트를 간단하게 방송할 때,)
	UFUNCTION(BlueprintCallable, Category = "Delegate")
	void Broadcast_OnSettingsOpen() { OnSettingsOpen.Broadcast(); }

	UFUNCTION(BlueprintCallable, Category = "Delegate")
	void Broadcast_OnSettingsClose() { OnSettingsClose.Broadcast(); }
};
