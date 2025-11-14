// AO_AIC_Werewolf.cpp

#include "AI/Controller/AO_AIC_Werewolf.h"
#include "AI/Character/AO_Enemy_Werewolf.h"
#include "Kismet/GameplayStatics.h"

AAO_AIC_Werewolf::AAO_AIC_Werewolf()
{
	// For Werewolf Settings
}

void AAO_AIC_Werewolf::BeginPlay()
{
	Super::BeginPlay();

	ControlledWerewolf = Cast<AAO_Enemy_Werewolf>(GetPawn());
	if(!ControlledWerewolf)
	{
		return;
	}
}

void AAO_AIC_Werewolf::HandleHowlEvent(const FVector& HowlLocation)
{
	if(GetLocalRole() != ROLE_Authority)
	{
		return;
	}

	if(!ControlledWerewolf)
	{
		ControlledWerewolf = Cast<AAO_Enemy_Werewolf>(GetPawn());
		if(!ControlledWerewolf)
		{
			return;
		}
	}

	UE_LOG(LogTemp, Log, TEXT("[WerewolfAI] HandleHowlEvent at %s"), *HowlLocation.ToString());

	// TODO: After Blackboards.
}
