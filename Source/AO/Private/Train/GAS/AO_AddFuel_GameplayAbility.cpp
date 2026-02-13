#include "Train/GAS/AO_AddFuel_GameplayAbility.h"
#include "GameFramework/GameStateBase.h"
#include "AbilitySystemComponent.h"
#include "GameplayTagContainer.h"

UAO_AddFuel_GameplayAbility::UAO_AddFuel_GameplayAbility()
{
	InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;

	FAbilityTriggerData TriggerData;
	TriggerData.TriggerTag = FGameplayTag::RequestGameplayTag(TEXT("Event.Interaction.AddFuel"));
	TriggerData.TriggerSource = EGameplayAbilityTriggerSource::GameplayEvent;

	AbilityTriggers.Add(TriggerData);
}

void UAO_AddFuel_GameplayAbility::ActivateAbility(
	const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo,
	const FGameplayAbilityActivationInfo ActivationInfo,
	const FGameplayEventData* TriggerEventData)
{
	if (!ActorInfo->IsNetAuthority() || !CommitAbility(Handle, ActorInfo, ActivationInfo))
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}
	UAbilitySystemComponent* ASC = ActorInfo->AbilitySystemComponent.Get();
	if (!ASC)
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}
	if (!AddEnergyEffectClass)
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}
	
	float FuelAmount = 0.0f;
	if (TriggerEventData)
	{
		float PlayerCountMultiplier = 1.0f;
		if (UWorld* World = GetWorld())
		{
			if (AGameStateBase* GS = World->GetGameState())
			{
				int32 CurrentPlayers = GS->PlayerArray.Num();
				PlayerCountMultiplier = (4-(CurrentPlayers))*0.2+1; 
			}
		}
		
		FuelAmount = TriggerEventData->EventMagnitude*PlayerCountMultiplier;
	}

	if (FuelAmount <= 0.0f)
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}

	FGameplayEffectSpecHandle Spec = ASC->MakeOutgoingSpec(AddEnergyEffectClass, 1.f, ASC->MakeEffectContext());
	if (Spec.IsValid() && Spec.Data.IsValid())
	{
		FGameplayTag SetByCallerTag = FGameplayTag::RequestGameplayTag(FName("Data.EnergyAmount")); 
       
		Spec.Data->SetSetByCallerMagnitude(SetByCallerTag, FuelAmount);

		ASC->ApplyGameplayEffectSpecToSelf(*Spec.Data.Get());
	}
	
	EndAbility(Handle, ActorInfo, ActivationInfo, false, false);
}