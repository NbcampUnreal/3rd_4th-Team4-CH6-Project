// HSJ : AO_InteractionGameplayAbility.cpp
#include "Interaction/GAS/Ability/AO_InteractionGameplayAbility.h"

#include "AbilitySystemComponent.h"
#include "AO_Log.h"

UAO_InteractionGameplayAbility::UAO_InteractionGameplayAbility()
{
	ActivationPolicy = ECYAbilityActivationPolicy::OnInputTriggered;
	InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;
}

void UAO_InteractionGameplayAbility::OnAvatarSet(const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilitySpec& Spec)
{
	Super::OnAvatarSet(ActorInfo, Spec);

	// OnSpawn 정책인 경우 부여 즉시 자동 활성화
	if (ActivationPolicy == ECYAbilityActivationPolicy::OnSpawn)
	{
		AO_LOG(LogHSJ, Log, TEXT("Ability %s - OnSpawn policy, attempting auto-activation"), 
			*GetClass()->GetName());
		
		bool bSuccess = ActorInfo->AbilitySystemComponent->TryActivateAbility(Spec.Handle, false);
		
		AO_LOG(LogHSJ, Log, TEXT("Auto-activation %s"), bSuccess ? TEXT("SUCCESS") : TEXT("FAILED"));
	}
}

AController* UAO_InteractionGameplayAbility::GetControllerFromActorInfo() const
{
	if (!CurrentActorInfo)
	{
		return nullptr;
	}

	if (AController* PC = CurrentActorInfo->PlayerController.Get())
	{
		return PC;
	}

	// Owner 체인을 따라 Controller 또는 Pawn의 Controller 탐색
	AActor* TestActor = CurrentActorInfo->OwnerActor.Get();
	while (TestActor)
	{
		// Controller 직접 발견
		if (AController* C = Cast<AController>(TestActor))
		{
			return C;
		}

		// Pawn 발견 시 해당 Pawn의 Controller 반환
		if (APawn* Pawn = Cast<APawn>(TestActor))
		{
			return Pawn->GetController();
		}

		// Owner 체인 계속 탐색
		TestActor = TestActor->GetOwner();
	}

	return nullptr;
}