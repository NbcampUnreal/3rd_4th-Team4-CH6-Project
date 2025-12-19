// AO_STCond_IsStunned.cpp

#include "AI/StateTree/Condition/AO_STCond_IsStunned.h"
#include "AI/Base/AO_AICharacterBase.h"
#include "AbilitySystemComponent.h"
#include "GameplayTagContainer.h"
#include "AIController.h"
#include "StateTreeExecutionContext.h"

bool FAO_STCond_IsStunned::TestCondition(FStateTreeExecutionContext& Context) const
{
	const FAO_STCond_IsStunned_InstanceData& InstanceData = Context.GetInstanceData<FAO_STCond_IsStunned_InstanceData>(*this);

	// AI 캐릭터가 유효해야 기절 상태 확인 가능
	AAO_AICharacterBase* AIChar = GetAICharacter(Context);
	if (!ensureMsgf(AIChar, TEXT("STCond_IsStunned: AICharacter is null")))
	{
		return InstanceData.bInvert;
	}

	bool bIsStunned = false;

	// GAS를 통해 기절 태그 확인
	UAbilitySystemComponent* ASC = AIChar->GetAbilitySystemComponent();
	if (ASC)
	{
		FGameplayTag StunnedTag = FGameplayTag::RequestGameplayTag(FName("Status.Debuff.Stunned"));
		bIsStunned = ASC->HasMatchingGameplayTag(StunnedTag);
	}

	return InstanceData.bInvert ? !bIsStunned : bIsStunned;
}

AAO_AICharacterBase* FAO_STCond_IsStunned::GetAICharacter(FStateTreeExecutionContext& Context) const
{
	if (AActor* Owner = Cast<AActor>(Context.GetOwner()))
	{
		if (AAO_AICharacterBase* AIChar = Cast<AAO_AICharacterBase>(Owner))
		{
			return AIChar;
		}
		if (AAIController* Controller = Cast<AAIController>(Owner))
		{
			return Cast<AAO_AICharacterBase>(Controller->GetPawn());
		}
	}
	return nullptr;
}
