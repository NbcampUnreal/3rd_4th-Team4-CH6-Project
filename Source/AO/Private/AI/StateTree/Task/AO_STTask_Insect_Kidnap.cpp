//KSJ : AO_STTask_Insect_Kidnap

#include "AI/StateTree/Task/AO_STTask_Insect_Kidnap.h"
#include "AI/Character/AO_Insect.h"
#include "AIController.h"
#include "AbilitySystemComponent.h"
#include "StateTreeExecutionContext.h"

EStateTreeRunStatus FAO_STTask_Insect_Kidnap::EnterState(FStateTreeExecutionContext& Context, const FStateTreeTransitionResult& Transition) const
{

	AAO_Insect* Insect = Cast<AAO_Insect>(Context.GetOwner());
	if (!Insect)
	{
		if (AAIController* AIC = Cast<AAIController>(Context.GetOwner()))
		{
			Insect = Cast<AAO_Insect>(AIC->GetPawn());
		}
	}

	if (!Insect)
	{
		return EStateTreeRunStatus::Failed;
	}

	// 이미 납치 중이면 성공
	if (Insect->IsKidnapping())
	{
		return EStateTreeRunStatus::Succeeded;
	}

	UAbilitySystemComponent* ASC = Insect->GetAbilitySystemComponent();
	if (!ASC)
	{
		return EStateTreeRunStatus::Failed;
	}

	// Ability 태그 결정 (잘못된 태그 검증)
	FGameplayTag Tag;
	if (KidnapAbilityTag.IsValid())
	{
		FString TagString = KidnapAbilityTag.ToString();
		// Status나 Debuff 태그는 Ability 태그가 아님 - 기본값 사용
		if (TagString.StartsWith(TEXT("Status.")) || TagString.StartsWith(TEXT("Debuff.")))
		{
			Tag = FGameplayTag::RequestGameplayTag(FName("Ability.Action.Kidnap"));
		}
		else
		{
			Tag = KidnapAbilityTag;
		}
	}
	else
	{
		Tag = FGameplayTag::RequestGameplayTag(FName("Ability.Action.Kidnap"));
	}
	
	
	// Ability가 존재하는지 먼저 확인
	TArray<FGameplayAbilitySpec*> MatchingAbilities;
	ASC->GetActivatableGameplayAbilitySpecsByAllMatchingTags(FGameplayTagContainer(Tag), MatchingAbilities);
	
	if (MatchingAbilities.Num() == 0)
	{
		return EStateTreeRunStatus::Failed;
	}
	
	
	if (ASC->TryActivateAbilitiesByTag(FGameplayTagContainer(Tag)))
	{
		return EStateTreeRunStatus::Running;
	}
	else
	{
		return EStateTreeRunStatus::Failed;
	}
}

EStateTreeRunStatus FAO_STTask_Insect_Kidnap::Tick(FStateTreeExecutionContext& Context, const float DeltaTime) const
{
	AAO_Insect* Insect = Cast<AAO_Insect>(Context.GetOwner());
	if (!Insect && Context.GetOwner()->IsA(AAIController::StaticClass()))
	{
		Insect = Cast<AAO_Insect>(Cast<AAIController>(Context.GetOwner())->GetPawn());
	}

	if (!Insect)
	{
		return EStateTreeRunStatus::Failed;
	}

	// 납치 성공 확인
	if (Insect->IsKidnapping())
	{
		return EStateTreeRunStatus::Succeeded;
	}

	// Ability가 활성화되어 있는지 확인
	UAbilitySystemComponent* ASC = Insect->GetAbilitySystemComponent();
	if (ASC)
	{
		// Ability 태그 결정 (EnterState와 동일한 로직)
		FGameplayTag Tag;
		if (KidnapAbilityTag.IsValid())
		{
			FString TagString = KidnapAbilityTag.ToString();
			if (TagString.StartsWith(TEXT("Status.")) || TagString.StartsWith(TEXT("Debuff.")))
			{
				Tag = FGameplayTag::RequestGameplayTag(FName("Ability.Action.Kidnap"));
			}
			else
			{
				Tag = KidnapAbilityTag;
			}
		}
		else
		{
			Tag = FGameplayTag::RequestGameplayTag(FName("Ability.Action.Kidnap"));
		}
		TArray<FGameplayAbilitySpec*> ActiveAbilities;
		ASC->GetActivatableGameplayAbilitySpecsByAllMatchingTags(FGameplayTagContainer(Tag), ActiveAbilities);
		
		// Ability가 활성화되어 있으면 계속 Running
		bool bAbilityActive = false;
		for (const FGameplayAbilitySpec* Spec : ActiveAbilities)
		{
			if (Spec && Spec->IsActive())
			{
				bAbilityActive = true;
				break;
			}
		}

		if (!bAbilityActive)
		{
			// Ability가 끝났는데 납치 상태가 아니면 실패
			return EStateTreeRunStatus::Failed;
		}
	}
	
	return EStateTreeRunStatus::Running;
}

