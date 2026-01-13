//KSJ : AO_GA_Werewolf_Howl


#include "AI/GAS/Ability/AO_GA_Werewolf_Howl.h"
#include "AI/Character/AO_Werewolf.h"
#include "AI/Component/AO_PackCoordComp.h"
#include "AI/Controller/AO_AggressiveAICtrl.h"
#include "Abilities/Tasks/AbilityTask_PlayMontageAndWait.h"
#include "Character/AO_PlayerCharacter.h"

UAO_GA_Werewolf_Howl::UAO_GA_Werewolf_Howl()
{
	InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;
	NetExecutionPolicy = EGameplayAbilityNetExecutionPolicy::ServerInitiated; 
}

void UAO_GA_Werewolf_Howl::ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData)
{
	Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);

	if (!CommitAbility(Handle, ActorInfo, ActivationInfo))
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}

	AAO_Werewolf* Wolf = Cast<AAO_Werewolf>(ActorInfo->AvatarActor.Get());
	if (!Wolf)
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}

	// Howl 전파 (결과는 Controller에서 활용)
	if (UAO_PackCoordComp* PackComp = Wolf->GetPackCoordComp())
	{
		if (AAO_AggressiveAICtrl* AIC = Cast<AAO_AggressiveAICtrl>(Wolf->GetController()))
		{
			FAO_HowlResult HowlResult = PackComp->BroadcastHowl(AIC->GetChaseTarget());
			
			// 동료가 없으면 포위 모드 해제 (일반 추격/공격으로)
			// 동료가 있으면 포위 모드로 진행 (이미 BroadcastHowl에서 처리됨)
			// StateTree에서 IsSurrounding 조건으로 분기 처리
		}
	}

	// 몽타주 재생
	if (HowlMontage)
	{
		UAbilityTask_PlayMontageAndWait* Task = UAbilityTask_PlayMontageAndWait::CreatePlayMontageAndWaitProxy(
			this, 
			TEXT("Howl"), 
			HowlMontage
		);

		Task->OnBlendOut.AddDynamic(this, &UAO_GA_Werewolf_Howl::OnMontageCompleted);
		Task->OnCompleted.AddDynamic(this, &UAO_GA_Werewolf_Howl::OnMontageCompleted);
		Task->OnInterrupted.AddDynamic(this, &UAO_GA_Werewolf_Howl::OnMontageCompleted);
		Task->OnCancelled.AddDynamic(this, &UAO_GA_Werewolf_Howl::OnMontageCompleted);
		
		Task->ReadyForActivation();
	}
	else
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, false);
	}
}

void UAO_GA_Werewolf_Howl::EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled)
{
	Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);
}

void UAO_GA_Werewolf_Howl::OnMontageCompleted()
{
	EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, false);
}
