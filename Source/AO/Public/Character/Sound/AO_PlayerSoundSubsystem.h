// AO_PlayerSoundSubsystem.h

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "Character/Customizing/AO_CustomizingComponent.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "AO_PlayerSoundSubsystem.generated.h"

class UAO_PlayerSoundDataAsset;

UCLASS()
class AO_API UAO_PlayerSoundSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()
	
public:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	
	UFUNCTION(BlueprintCallable, Category = "Sound")
	USoundBase* GetSound(ECharacterMesh MeshType, FGameplayTag SoundTag) const;

	UFUNCTION(BlueprintCallable, Category = "Sound")
	USoundBase* GetSoundFromActor(const AActor* Actor, FGameplayTag SoundTag) const;

private:
	UPROPERTY(Transient)
	TObjectPtr<UAO_PlayerSoundDataAsset> LoadedDA = nullptr;

	UPROPERTY(Transient)
	TMap<FGameplayTag, TObjectPtr<USoundBase>> LoadedDefaultSounds;
	
	UAO_PlayerSoundDataAsset* GetDataAsset() const;
	ECharacterMesh GetMeshTypeSafe(const AActor* Actor) const;
};
