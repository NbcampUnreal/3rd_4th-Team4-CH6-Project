// AO_PlayerSoundSubsystem.cpp

#include "Character/Sound/AO_PlayerSoundSubsystem.h"
#include "Character/Sound/AO_PlayerSoundDataAsset.h"
#include "Character/Sound/AO_PlayerSoundSettings.h"

void UAO_PlayerSoundSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

	const UAO_PlayerSoundSettings* Settings = GetDefault<UAO_PlayerSoundSettings>();
	if (!ensure(Settings))
	{
		return;
	}

	if (!Settings->CharacterSoundDataAsset.IsNull())
	{
		CharacterSoundDataAsset = Settings->CharacterSoundDataAsset;
		LoadedDA = CharacterSoundDataAsset.LoadSynchronous();
	}

	if (!Settings->DefaultNotEnoughStaminaSound.IsNull())
	{
		DefaultNotEnoughStaminaSound = Settings->DefaultNotEnoughStaminaSound;
		LoadedDefaultNotEnoughStaminaSound = DefaultNotEnoughStaminaSound.LoadSynchronous();
	}

	if (!Settings->DefaultDamageReactSound.IsNull())
	{
		DefaultNotEnoughStaminaSound = Settings->DefaultDamageReactSound;
		LoadedDefaultDamageReactSound = DefaultDamageReactSound.LoadSynchronous();
	}
}

USoundBase* UAO_PlayerSoundSubsystem::GetNotEnoughStaminaSound(ECharacterMesh MeshType) const
{
	if (UAO_PlayerSoundDataAsset* DA = GetDataAsset())
	{
		FCharacterSoundSet Set;
		if (DA->TryGetSoundSet(MeshType, Set))
		{
			if (Set.NotEnoughStamina)
			{
				return Set.NotEnoughStamina;
			}
		}
	}

	return LoadedDefaultNotEnoughStaminaSound;
}

USoundBase* UAO_PlayerSoundSubsystem::GetNotEnoughStaminaSoundFromActor(const AActor* Actor) const
{
	if (!Actor)
	{
		return LoadedDefaultNotEnoughStaminaSound;
	}

	if (const UAO_CustomizingComponent* CustomizingComp = Actor->FindComponentByClass<UAO_CustomizingComponent>())
	{
		const ECharacterMesh MeshType = CustomizingComp->GetCustomizingData().CharacterMeshType;
		return GetNotEnoughStaminaSound(MeshType);
	}

	return LoadedDefaultNotEnoughStaminaSound;
}

USoundBase* UAO_PlayerSoundSubsystem::GetDamageReactSound(ECharacterMesh MeshType) const
{
	if (UAO_PlayerSoundDataAsset* DA = GetDataAsset())
	{
		FCharacterSoundSet Set;
		if (DA->TryGetSoundSet(MeshType, Set))
		{
			if (Set.DamageReact)
			{
				return Set.DamageReact;
			}
		}
	}

	return LoadedDefaultDamageReactSound;
}

USoundBase* UAO_PlayerSoundSubsystem::GetDamageReactSoundFromActor(const AActor* Actor) const
{
	if (!Actor)
	{
		return LoadedDefaultDamageReactSound;
	}

	if (const UAO_CustomizingComponent* CustomizingComp = Actor->FindComponentByClass<UAO_CustomizingComponent>())
	{
		const ECharacterMesh MeshType = CustomizingComp->GetCustomizingData().CharacterMeshType;
		return GetDamageReactSound(MeshType);
	}

	return LoadedDefaultDamageReactSound;
}

UAO_PlayerSoundDataAsset* UAO_PlayerSoundSubsystem::GetDataAsset() const
{
	return LoadedDA;
}
