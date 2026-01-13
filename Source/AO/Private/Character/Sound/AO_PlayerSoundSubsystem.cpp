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

	// DeveloperSettings에서 설정한 데이터 로드
	if (!Settings->CharacterSoundDataAsset.IsNull())
	{
		LoadedDA = Settings->CharacterSoundDataAsset.LoadSynchronous();
	}

	for (const FDefaultSoundEntry& Entry : Settings->DefaultSounds)
	{
		if (Entry.Sound.IsValid() && !Entry.Sound.IsNull())
		{
			LoadedDefaultSounds.Add(Entry.SoundTag, Entry.Sound.LoadSynchronous());
		}
	}
}

UAO_PlayerSoundDataAsset* UAO_PlayerSoundSubsystem::GetDataAsset() const
{
	return LoadedDA;
}

USoundBase* UAO_PlayerSoundSubsystem::GetSound(ECharacterMesh MeshType, FGameplayTag SoundTag) const
{
	if (UAO_PlayerSoundDataAsset* DataAsset = GetDataAsset())
	{
		if (USoundBase* Sound = DataAsset->FindSound(MeshType, SoundTag))
		{
			return Sound;
		}
	}

	if (const TObjectPtr<USoundBase>* Sound = LoadedDefaultSounds.Find(SoundTag))
	{
		return Sound->Get();
	}

	return nullptr;
}

USoundBase* UAO_PlayerSoundSubsystem::GetSoundFromActor(const AActor* Actor, FGameplayTag SoundTag) const
{
	const ECharacterMesh MeshType = GetMeshTypeSafe(Actor);
	return GetSound(MeshType, SoundTag);
}

ECharacterMesh UAO_PlayerSoundSubsystem::GetMeshTypeSafe(const AActor* Actor) const
{
	if (!Actor)
	{
		return ECharacterMesh::Elsa;	
	}
	
	if (const UAO_CustomizingComponent* CustomizingComp = Actor->FindComponentByClass<UAO_CustomizingComponent>())
	{
		return CustomizingComp->GetCustomizingData().CharacterMeshType;
	}
	
	return ECharacterMesh::Elsa;
}
