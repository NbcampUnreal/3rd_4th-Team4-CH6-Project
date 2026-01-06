// AO_PlayerSoundDataAsset.cpp


#include "Character/Sound/AO_PlayerSoundDataAsset.h"

bool UAO_PlayerSoundDataAsset::TryGetSoundSet(ECharacterMesh MeshType, FCharacterSoundSet& OutSet) const
{
	if (const FCharacterSoundSet* Found = SoundSetByMesh.Find(MeshType))
	{
		OutSet = *Found;
		return true;
	}

	return false;
}
