//KSJ : AO_STTask_Wolf_Howl


#include "AI/StateTree/Task/AO_STTask_Wolf_Howl.h"
#include "AI/Character/AO_Werewolf.h"
#include "AI/Controller/AO_WerewolfController.h"
#include "AI/Component/AO_PackCoordComp.h"
#include "StateTreeExecutionContext.h"
#include "Character/AO_PlayerCharacter.h"
#include "AIController.h"
#include "AbilitySystemComponent.h"

EStateTreeRunStatus FAO_STTask_Wolf_Howl::EnterState(FStateTreeExecutionContext& Context, const FStateTreeTransitionResult& Transition) const
{
	FInstanceDataType& InstanceData = Context.GetInstanceData<FInstanceDataType>(*this);
	
	AActor* OwnerActor = Cast<AActor>(Context.GetOwner());
	AAO_Werewolf* Wolf = nullptr;
	AAO_WerewolfController* WolfCtrl = nullptr;

	if (APawn* Pawn = Cast<APawn>(OwnerActor))
	{
		Wolf = Cast<AAO_Werewolf>(Pawn);
		WolfCtrl = Cast<AAO_WerewolfController>(Pawn->GetController());
	}
	else if (AController* Controller = Cast<AController>(OwnerActor))
	{
		WolfCtrl = Cast<AAO_WerewolfController>(Controller);
		Wolf = Cast<AAO_Werewolf>(Controller->GetPawn());
	}

	InstanceData.Controller = WolfCtrl;

	if (Wolf)
	{
		InstanceData.PackComp = Wolf->GetPackCoordComp();
	}

	if (!InstanceData.PackComp || !WolfCtrl || !Wolf)
	{
		return EStateTreeRunStatus::Failed;
	}

	// Howl 상태 마킹 (Controller에서 관리)
	// 이렇게 하면 다음에 플레이어를 발견해도 Howl State에 다시 진입하지 않음
	WolfCtrl->MarkHowledOrJoined();

	// 이동 정지
	WolfCtrl->StopMovement();

	// GAS Ability 실행 (Howl Ability - 몽타주 + BroadcastHowl)
	UAbilitySystemComponent* ASC = Wolf->GetAbilitySystemComponent();
	if (ASC)
	{
		FGameplayTag HowlTag = FGameplayTag::RequestGameplayTag(FName("Ability.Action.Howl"));
		if (HowlTag.IsValid())
		{
			FGameplayTagContainer TagContainer;
			TagContainer.AddTag(HowlTag);
			ASC->TryActivateAbilitiesByTag(TagContainer);
		}
	}

	InstanceData.Timer = 0.f;

	return EStateTreeRunStatus::Running;
}

EStateTreeRunStatus FAO_STTask_Wolf_Howl::Tick(FStateTreeExecutionContext& Context, const float DeltaTime) const
{
	FInstanceDataType& InstanceData = Context.GetInstanceData<FInstanceDataType>(*this);

	InstanceData.Timer += DeltaTime;

	// 애니메이션 시간만큼 대기 후 성공 처리
	if (InstanceData.Timer >= InstanceData.HowlDuration)
	{
		return EStateTreeRunStatus::Succeeded;
	}

	return EStateTreeRunStatus::Running;
}
