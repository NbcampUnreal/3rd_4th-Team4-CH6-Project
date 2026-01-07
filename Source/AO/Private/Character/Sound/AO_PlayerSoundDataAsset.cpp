// AO_PlayerSoundDataAsset.cpp


#include "Character/Sound/AO_PlayerSoundDataAsset.h"

USoundBase* UAO_PlayerSoundDataAsset::FindSound(ECharacterMesh MeshType, FGameplayTag SoundTag) const
{
	if (const FCharacterSoundSet* FoundSet = SoundSetByMesh.Find(MeshType))
	{
		if (const TObjectPtr<USoundBase>* Sound = FoundSet->Sounds.Find(SoundTag))
		{
			return Sound->Get();
		}
	}

	return nullptr;
}
