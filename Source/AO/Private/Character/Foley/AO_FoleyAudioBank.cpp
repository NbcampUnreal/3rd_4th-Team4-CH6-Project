// AO_FoleyAudioBank.cpp

#include "Character/Foley/AO_FoleyAudioBank.h"

#include "AO_Log.h"

USoundBase* UAO_FoleyAudioBank::GetSoundFromFoleyEvent(const FGameplayTag& Event)
{
	TObjectPtr<USoundBase> FoleySound = Assets.FindRef(Event);
	if (!FoleySound)
	{
		AO_LOG(LogKH, Warning, TEXT("Failed to find FoleySound for Event: %s"), *Event.ToString());
		return nullptr;
	}
	
	return FoleySound;
}
