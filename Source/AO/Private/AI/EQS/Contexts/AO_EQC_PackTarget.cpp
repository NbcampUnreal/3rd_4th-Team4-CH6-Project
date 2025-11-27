// KSJ : AO_EQC_PackTarget.cpp

#include "AI/EQS/Contexts/AO_EQC_PackTarget.h"
#include "AI/Controller/AO_AIC_Werewolf.h"
#include "EnvironmentQuery/EnvQueryTypes.h"
#include "EnvironmentQuery/Items/EnvQueryItemType_Actor.h"

void UAO_EQC_PackTarget::ProvideContext(FEnvQueryInstance& QueryInstance, FEnvQueryContextData& ContextData) const
{
	AActor* Querier = Cast<AActor>(QueryInstance.Owner.Get());
	AAO_AIC_Werewolf* WolfAIC = Querier ? Cast<AAO_AIC_Werewolf>(Querier->GetInstigatorController()) : nullptr;

	if (WolfAIC)
	{
		AActor* Target = WolfAIC->GetPackTargetPlayer();
		if (Target)
		{
			UEnvQueryItemType_Actor::SetContextHelper(ContextData, Target);
		}
	}
}

