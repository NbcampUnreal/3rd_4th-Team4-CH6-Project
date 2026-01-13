// AO_FoleyAudioBankInterface.h

#pragma once

#include "CoreMinimal.h"
#include "AO_FoleyAudioBank.h"
#include "UObject/Interface.h"
#include "AO_FoleyAudioBankInterface.generated.h"

UINTERFACE(MinimalAPI, BlueprintType)
class UAO_FoleyAudioBankInterface : public UInterface
{
	GENERATED_BODY()
};

class AO_API IAO_FoleyAudioBankInterface
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Foley")
	UAO_FoleyAudioBank* GetFoleyAudioBank() const;
	
	virtual UAO_FoleyAudioBank* GetFoleyAudioBank_Implementation() const = 0;

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Foley")
	bool CanPlayFootstepSounds() const;
	
	virtual bool CanPlayFootstepSounds_Implementation() const = 0;
};
