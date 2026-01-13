// HSJ : GA_Interact_PuzzleElement.cpp
#include "Puzzle/GAS/Ability/GA_Interact_PuzzleElement.h"
#include "AO_Log.h"
#include "Interaction/GAS/Tag/AO_InteractionGameplayTags.h"
#include "Puzzle/Element/AO_PuzzleElement.h"

UGA_Interact_PuzzleElement::UGA_Interact_PuzzleElement()
{
	InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;
	NetExecutionPolicy = EGameplayAbilityNetExecutionPolicy::ServerOnly;
	
	// Finalize 이벤트로 자동 트리거되도록 설정
	if (HasAnyFlags(RF_ClassDefaultObject))
	{
		FAbilityTriggerData TriggerData;
		TriggerData.TriggerTag = AO_InteractionTags::Ability_Action_AbilityInteract_Finalize;
		TriggerData.TriggerSource = EGameplayAbilityTriggerSource::GameplayEvent;
		AbilityTriggers.Add(TriggerData);
	}
}

void UGA_Interact_PuzzleElement::ActivateAbility(
	const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo,
	const FGameplayAbilityActivationInfo ActivationInfo,
	const FGameplayEventData* TriggerEventData)
{
	if (!CommitAbility(Handle, ActorInfo, ActivationInfo))
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}

	Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);

	// TriggerEventData에서 Target(PuzzleElement) 추출
	if (!TriggerEventData)
	{
		AO_LOG(LogHSJ, Warning, TEXT("TriggerEventData is null"));
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}

	TObjectPtr<AAO_PuzzleElement> PuzzleElement = Cast<AAO_PuzzleElement>(const_cast<AActor*>(TriggerEventData->Target.Get()));
	if (!PuzzleElement)
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}

	// PuzzleElement의 상호작용 로직 실행
	TObjectPtr<AActor> Instigator = const_cast<AActor*>(TriggerEventData->Instigator.Get());
	PuzzleElement->OnInteractionSuccess(Instigator);
	
	EndAbility(Handle, ActorInfo, ActivationInfo, true, false);
}