// AO_PlayerSoundDataAsset.h

#pragma once

#include "CoreMinimal.h"
#include "Character/Customizing/AO_CustomizingComponent.h"
#include "Engine/DataAsset.h"
#include "AO_PlayerSoundDataAsset.generated.h"

USTRUCT(BlueprintType)
struct FCharacterSoundSet
{
	GENERATED_BODY()

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Sound")
	TObjectPtr<USoundBase> NotEnoughStamina = nullptr;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Sound")
	TObjectPtr<USoundBase> DamageReact = nullptr;
};

UCLASS()
class AO_API UAO_PlayerSoundDataAsset : public UDataAsset
{
	GENERATED_BODY()

public:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Sound")
	TMap<ECharacterMesh, FCharacterSoundSet> SoundSetByMesh;

	UFUNCTION(BlueprintCallable, Category = "Sound")
	bool TryGetSoundSet(ECharacterMesh MeshType, FCharacterSoundSet& OutSet) const;
};
