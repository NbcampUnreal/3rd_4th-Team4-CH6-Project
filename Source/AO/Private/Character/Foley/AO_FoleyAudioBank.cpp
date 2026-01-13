// AO_FoleyAudioBank.cpp

#include "Character/Foley/AO_FoleyAudioBank.h"

#include "AO_Log.h"

USoundBase* UAO_FoleyAudioBank::GetSoundFromFoleyEvent(const FGameplayTag& Event)
{
	TObjectPtr<USoundBase> FoleySound = Assets.FindRef(Event);
	checkf(FoleySound, TEXT("FoleySound is null"));
	
	return FoleySound;
}
