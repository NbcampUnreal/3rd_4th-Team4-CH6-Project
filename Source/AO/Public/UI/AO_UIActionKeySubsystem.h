// JSH: AO_UIActionKeySubsystem.h

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "InputCoreTypes.h"
#include "AO_UIActionKeySubsystem.generated.h"

class UInputAction;
class UInputMappingContext;

UCLASS()
class AO_API UAO_UIActionKeySubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;

	bool IsUIOpenKey(const FKey& Key) const;
	bool IsUICloseKey(const FKey& Key) const;
	
public:
	UInputMappingContext* GetUIIMC() const;
	UInputAction* GetUIOpenAction() const;
	UInputAction* GetUICloseAction() const;
	
private:
	UPROPERTY()
	TObjectPtr<UInputMappingContext> CachedIMC = nullptr;

	UPROPERTY()
	TObjectPtr<UInputAction> CachedOpenAction = nullptr;

	UPROPERTY()
	TObjectPtr<UInputAction> CachedCloseAction = nullptr;

private:
	void RefreshCache();

private:
	UPROPERTY()
	TArray<FKey> CachedOpenKeys;

	UPROPERTY()
	TArray<FKey> CachedCloseKeys;

private:
	static void AddUniqueKey(TArray<FKey>& Keys, const FKey& Key);
};
