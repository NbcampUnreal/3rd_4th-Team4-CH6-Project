// AO_CameraProfile.cpp

#include "Player/Camera/AO_CameraProfile.h"

const FAO_CameraSettings* UAO_CameraProfile::FindByTag(const FGameplayTag& Tag) const
{
	for (const auto& P : Profiles)
	{
		if (P.Tag == Tag) return &P;
	}

	return nullptr;
}
