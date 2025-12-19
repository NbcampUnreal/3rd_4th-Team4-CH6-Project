// AO_STTask_Insect_Kidnap.cpp

#include "AI/StateTree/Task/AO_STTask_Insect_Kidnap.h"
#include "AI/Character/AO_Insect.h"
#include "AIController.h"
#include "AbilitySystemComponent.h"
#include "StateTreeExecutionContext.h"
#include "AO_Log.h"

EStateTreeRunStatus FAO_STTask_Insect_Kidnap::EnterState(FStateTreeExecutionContext& Context, const FStateTreeTransitionResult& Transition) const
{
	AO_LOG(LogKSJ, Log, TEXT("AO_STTask_Insect_Kidnap: EnterState Called"));

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
		AO_LOG(LogKSJ, Error, TEXT("AO_STTask_Insect_Kidnap: Insect is null"));
		return EStateTreeRunStatus::Failed;
	}

	// 이미 납치 중이면 성공
	if (Insect->IsKidnapping())
	{
		AO_LOG(LogKSJ, Log, TEXT("AO_STTask_Insect_Kidnap: Already kidnapping, returning Succeeded"));
		return EStateTreeRunStatus::Succeeded;
	}

	UAbilitySystemComponent* ASC = Insect->GetAbilitySystemComponent();
	if (!ASC)
	{
		AO_LOG(LogKSJ, Error, TEXT("AO_STTask_Insect_Kidnap: AbilitySystemComponent is null"));
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
			AO_LOG(LogKSJ, Warning, TEXT("AO_STTask_Insect_Kidnap: Invalid ability tag detected (%s), using default Ability.Action.Kidnap"), *TagString);
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
	
	AO_LOG(LogKSJ, Log, TEXT("AO_STTask_Insect_Kidnap: Attempting to activate ability with tag: %s"), *Tag.ToString());
	
	// Ability가 존재하는지 먼저 확인
	TArray<FGameplayAbilitySpec*> MatchingAbilities;
	ASC->GetActivatableGameplayAbilitySpecsByAllMatchingTags(FGameplayTagContainer(Tag), MatchingAbilities);
	
	if (MatchingAbilities.Num() == 0)
	{
		AO_LOG(LogKSJ, Error, TEXT("AO_STTask_Insect_Kidnap: No ability found with tag: %s. Check if Ability is granted to Insect."), *Tag.ToString());
		return EStateTreeRunStatus::Failed;
	}
	
	AO_LOG(LogKSJ, Log, TEXT("AO_STTask_Insect_Kidnap: Found %d ability(ies) with tag: %s"), MatchingAbilities.Num(), *Tag.ToString());
	
	if (ASC->TryActivateAbilitiesByTag(FGameplayTagContainer(Tag)))
	{
		AO_LOG(LogKSJ, Log, TEXT("AO_STTask_Insect_Kidnap: Ability activated successfully"));
		return EStateTreeRunStatus::Running;
	}
	else
	{
		AO_LOG(LogKSJ, Warning, TEXT("AO_STTask_Insect_Kidnap: Failed to activate ability with tag: %s (Ability may be on cooldown or blocked)"), *Tag.ToString());
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
		AO_LOG(LogKSJ, Error, TEXT("AO_STTask_Insect_Kidnap::Tick: Insect is null"));
		return EStateTreeRunStatus::Failed;
	}

	// 납치 성공 확인
	if (Insect->IsKidnapping())
	{
		AO_LOG(LogKSJ, Log, TEXT("AO_STTask_Insect_Kidnap::Tick: Kidnap successful!"));
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
			AO_LOG(LogKSJ, Warning, TEXT("AO_STTask_Insect_Kidnap::Tick: Ability ended but not kidnapping, returning Failed"));
			return EStateTreeRunStatus::Failed;
		}
	}
	
	return EStateTreeRunStatus::Running;
}

