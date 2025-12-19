// Fill out your copyright notice in the Description page of Project Settings.


#include "AI/EQS/AO_EQS_Context_Target.h"
#include "AI/Controller/AO_AggressiveAICtrl.h"
#include "Character/AO_PlayerCharacter.h"
#include "EnvironmentQuery/EnvQueryTypes.h"
#include "EnvironmentQuery/Items/EnvQueryItemType_Actor.h"

void UAO_EQS_Context_Target::ProvideContext(FEnvQueryInstance& QueryInstance, FEnvQueryContextData& ContextData) const
{
	AActor* QueryOwner = Cast<AActor>(QueryInstance.Owner.Get());
	if (!QueryOwner) return;

	AAO_AggressiveAICtrl* AIC = nullptr;

	if (APawn* Pawn = Cast<APawn>(QueryOwner))
	{
		AIC = Cast<AAO_AggressiveAICtrl>(Pawn->GetController());
	}
	else
	{
		AIC = Cast<AAO_AggressiveAICtrl>(QueryOwner);
	}

	if (AIC)
	{
		AAO_PlayerCharacter* Target = AIC->GetChaseTarget();
		if (Target)
		{
			UEnvQueryItemType_Actor::SetContextHelper(ContextData, Target);
		}
	}
}
