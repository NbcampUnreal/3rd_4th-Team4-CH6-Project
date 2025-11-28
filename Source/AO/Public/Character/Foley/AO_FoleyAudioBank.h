// AO_FoleyAudioBank.h

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "GameplayTagContainer.h"
#include "AO_FoleyAudioBank.generated.h"

UCLASS()
class AO_API UAO_FoleyAudioBank : public UPrimaryDataAsset
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, Category = "Foley")
	USoundBase* GetSoundFromFoleyEvent(const FGameplayTag& Event);
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Foley")
	TMap<FGameplayTag, TObjectPtr<USoundBase>> Assets;
};
