// AO_AIC_Troll.cpp

#include "AI/Controller/AO_AIC_Troll.h"
#include "AI/Character/AO_Enemy_Troll.h"

AAO_AIC_Troll::AAO_AIC_Troll()
{
	// for Troll Settings
}

void AAO_AIC_Troll::BeginPlay()
{
	Super::BeginPlay();

	ControlledTroll = Cast<AAO_Enemy_Troll>(GetPawn());
	if(!ControlledTroll)
	{
		return;
	}

	// TODO: Troll Action Logics (ex: Fury.. or Specific Gimmick)
}
