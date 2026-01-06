// AO_PlayerSoundSubsystem.h

#pragma once

#include "CoreMinimal.h"
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

	// 캐릭터 메시별 사운드 데이터
	UPROPERTY(EditDefaultsOnly, Category = "Sound")
	TSoftObjectPtr<UAO_PlayerSoundDataAsset> CharacterSoundDataAsset;

	UPROPERTY(EditDefaultsOnly, Category = "Sound")
	TSoftObjectPtr<USoundBase> DefaultNotEnoughStaminaSound;
	
	UPROPERTY(EditDefaultsOnly, Category = "Sound")
	TSoftObjectPtr<USoundBase> DefaultDamageReactSound;

	UFUNCTION(BlueprintCallable, Category = "Sound")
	USoundBase* GetNotEnoughStaminaSound(ECharacterMesh MeshType) const;

	UFUNCTION(BlueprintCallable, Category = "Sound")
	USoundBase* GetNotEnoughStaminaSoundFromActor(const AActor* Actor) const;

	UFUNCTION(BlueprintCallable, Category = "Sound")
	USoundBase* GetDamageReactSound(ECharacterMesh MeshType) const;

	UFUNCTION(BlueprintCallable, Category = "Sound")
	USoundBase* GetDamageReactSoundFromActor(const AActor* Actor) const;

private:
	UPROPERTY(Transient)
	TObjectPtr<UAO_PlayerSoundDataAsset> LoadedDA = nullptr;

	UPROPERTY(Transient)
	TObjectPtr<USoundBase> LoadedDefaultNotEnoughStaminaSound = nullptr;

	UPROPERTY(Transient)
	TObjectPtr<USoundBase> LoadedDefaultDamageReactSound = nullptr;
	
	UAO_PlayerSoundDataAsset* GetDataAsset() const;
};
