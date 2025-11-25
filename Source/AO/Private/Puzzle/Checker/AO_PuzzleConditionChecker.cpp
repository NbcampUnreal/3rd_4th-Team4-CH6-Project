// HSJ : AO_PuzzleConditionChecker.cpp
#include "Puzzle/Checker/AO_PuzzleConditionChecker.h"
#include "Net/UnrealNetwork.h"
#include "AbilitySystemInterface.h"
#include "AbilitySystemComponent.h"
#include "AO_Log.h"
#include "Puzzle/Interface/AO_Interface_PuzzleElement.h"

AAO_PuzzleConditionChecker::AAO_PuzzleConditionChecker()
{
	bReplicates = true;
	bAlwaysRelevant = true;
	SetReplicateMovement(false);
}

void AAO_PuzzleConditionChecker::BeginPlay()
{
	Super::BeginPlay();
}

void AAO_PuzzleConditionChecker::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	Super::EndPlay(EndPlayReason);
	
	ClearAllTimers();
}

void AAO_PuzzleConditionChecker::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	
	DOREPLIFETIME(AAO_PuzzleConditionChecker, ActiveEvents);
	DOREPLIFETIME(AAO_PuzzleConditionChecker, bIsCompleted);
}

void AAO_PuzzleConditionChecker::OnPuzzleEvent(FGameplayTag EventTag, AActor* EventInstigator)
{
	if (!HasAuthority() || !EventTag.IsValid()) return;

	// 일회성 완료이고 이미 완료됨
	if (bOneTimeCompletion && bIsCompleted) return;

	// 이미 활성화된 태그면 무시
	if (ActiveEvents.HasTag(EventTag)) return;

	// 태그 활성화
	ActiveEvents.AddTag(EventTag);
	//AO_LOG_NET(LogHSJ, Log, TEXT("Event added: %s (Total: %d)"), *EventTag.ToString(), ActiveEvents.Num());

	// 순서 조건용 히스토리 업데이트
	for (int32 i = 0; i < Conditions.Num(); ++i)
	{
		const FPuzzleCondition& Condition = Conditions[i];
		
		// 순서가 필요한 조건이면 히스토리에 추가
		if (Condition.bRequiresOrder && Condition.OrderedTags.Num() > 0)
		{
			if (!ConditionOrderHistory.Contains(i))
			{
				ConditionOrderHistory.Add(i, TArray<FGameplayTag>());
			}
			ConditionOrderHistory[i].Add(EventTag);
		}

		// 시간 제한이 있으면 타이머 시작
		if (Condition.TimeLimit > 0.0f)
		{
			StartConditionTimer(i, Condition.TimeLimit);
		}
	}

	// 조건 검사
	if (CheckAllConditions())
	{
		CompletePuzzle();
	}
	else
	{
		// 실패 조건 체크, 모든 필수 입력은 받았는데 조건 불충족
		if (ShouldFailPuzzle())
		{
			FailPuzzle();
		}
	}
}

void AAO_PuzzleConditionChecker::RemovePuzzleEvent(FGameplayTag EventTag)
{
	if (!HasAuthority() || !EventTag.IsValid()) return;

	ActiveEvents.RemoveTag(EventTag);
	//AO_LOG_NET(LogHSJ, Log, TEXT("Event removed: %s"), *EventTag.ToString());
}

bool AAO_PuzzleConditionChecker::CheckAllConditions()
{
	if (Conditions.Num() == 0) return false;

	int32 MetConditionsCount = 0;

	for (int32 i = 0; i < Conditions.Num(); ++i)
	{
		if (CheckSingleCondition(Conditions[i], i))
		{
			MetConditionsCount++;
		}
	}

	// AND 조건: 모두 만족
	if (bRequireAllConditions)
	{
		return MetConditionsCount == Conditions.Num();
	}
	// OR 조건: 하나라도 만족
	else
	{
		return MetConditionsCount > 0;
	}
}

bool AAO_PuzzleConditionChecker::CheckSingleCondition(const FPuzzleCondition& Condition, int32 ConditionIndex)
{
	// BlockingTags 체크 (하나라도 있으면 실패)
	if (Condition.BlockingTags.Num() > 0)
	{
		if (ActiveEvents.HasAny(Condition.BlockingTags))
		{
			return false;
		}
	}

	// 순서 조건 체크
	if (Condition.bRequiresOrder && Condition.OrderedTags.Num() > 0)
	{
		if (!ConditionOrderHistory.Contains(ConditionIndex))
		{
			return false;
		}

		const TArray<FGameplayTag>& History = ConditionOrderHistory[ConditionIndex];
		
		// 히스토리가 부족함
		if (History.Num() < Condition.OrderedTags.Num())
		{
			return false;
		}

		// 최근 N개 태그가 OrderedTags와 순서대로 일치하는지 체크
		int32 StartIndex = History.Num() - Condition.OrderedTags.Num();
		for (int32 i = 0; i < Condition.OrderedTags.Num(); ++i)
		{
			if (!History[StartIndex + i].MatchesTagExact(Condition.OrderedTags[i]))
			{
				return false;
			}
		}

		return true;
	}
	
	// RequiredTags 체크 (모두 활성화되어야 함)
	if (Condition.RequiredTags.Num() > 0)
	{
		return ActiveEvents.HasAll(Condition.RequiredTags);
	}

	// 아무 조건도 없으면 실패
	return false;
}

void AAO_PuzzleConditionChecker::CompletePuzzle()
{
	if (!HasAuthority()) return;

	// 일회성이고 이미 완료됨
	if (bOneTimeCompletion && bIsCompleted) return;

	AO_LOG_NET(LogHSJ, Warning, TEXT("Puzzle completed!"));

	bIsCompleted = true;
	ClearAllTimers();

	// CompletionTag를 TargetActors에 부여
	if (CompletionTag.IsValid())
	{
		for (AActor* TargetActor : TargetActors)
		{
			if (!TargetActor) continue;

			IAbilitySystemInterface* ASI = Cast<IAbilitySystemInterface>(TargetActor);
			if (ASI && ASI->GetAbilitySystemComponent())
			{
				UAbilitySystemComponent* ASC = ASI->GetAbilitySystemComponent();
				ASC->AddLooseGameplayTag(CompletionTag);
				
				//AO_LOG_NET(LogHSJ, Log, TEXT("Added completion tag to: %s"), *TargetActor->GetName());
			}
		}
	}

	// 퍼즐 성공 시 요소들 비활성화
	if (bDisableElementsOnComplete)
	{
		DisableLinkedElements();
	}

	OnPuzzleCompleted.Broadcast();

}

void AAO_PuzzleConditionChecker::FailPuzzle()
{
	if (!HasAuthority()) return;
	
	AO_LOG_NET(LogHSJ, Warning, TEXT("Puzzle failed!"));
	
	ClearAllTimers();
	MulticastPuzzleFailed();
	ResetPuzzle();
}

void AAO_PuzzleConditionChecker::ResetPuzzle()
{
	if (!HasAuthority()) return;

	ActiveEvents.Reset();
	ConditionOrderHistory.Reset();
	bIsCompleted = false;
	ClearAllTimers();

	// 연결된 요소들도 리셋
	ResetLinkedElements();
}

float AAO_PuzzleConditionChecker::GetCompletionProgress() const
{
	if (Conditions.Num() == 0) return 0.0f;

	int32 TotalRequiredTags = 0;
	int32 AcquiredTags = 0;

	// 모든 조건의 RequiredTags 수집
	for (const FPuzzleCondition& Condition : Conditions)
	{
		TotalRequiredTags += Condition.RequiredTags.Num();
		
		for (const FGameplayTag& RequiredTag : Condition.RequiredTags)
		{
			if (ActiveEvents.HasTag(RequiredTag))
			{
				AcquiredTags++;
			}
		}
	}

	if (TotalRequiredTags == 0) return 0.0f;

	return static_cast<float>(AcquiredTags) / static_cast<float>(TotalRequiredTags);
}

void AAO_PuzzleConditionChecker::StartConditionTimer(int32 ConditionIndex, float Duration)
{
	if (Duration <= 0.0f) return;

	FTimerHandle& TimerHandle = ConditionTimerHandles.FindOrAdd(ConditionIndex);
	
	GetWorld()->GetTimerManager().SetTimer(
		TimerHandle,
		FTimerDelegate::CreateUObject(this, &AAO_PuzzleConditionChecker::OnConditionTimeout, ConditionIndex),
		Duration,
		false
	);
}

void AAO_PuzzleConditionChecker::OnConditionTimeout(int32 ConditionIndex)
{
	AO_LOG_NET(LogHSJ, Warning, TEXT("Condition %d timeout"), ConditionIndex);
	FailPuzzle();
}

void AAO_PuzzleConditionChecker::OnRep_IsCompleted()
{
	if (bIsCompleted)
	{
		OnPuzzleCompleted.Broadcast();
	}
}

void AAO_PuzzleConditionChecker::OnRep_ActiveEvents()
{
	OnPuzzleProgress.Broadcast(GetCompletionProgress());
}

void AAO_PuzzleConditionChecker::MulticastPuzzleFailed_Implementation()
{
	OnPuzzleFailed.Broadcast();
}

void AAO_PuzzleConditionChecker::ClearAllTimers()
{
	if (UWorld* World = GetWorld())
	{
		TArray<int32> TimerKeys;
		ConditionTimerHandles.GetKeys(TimerKeys);
		
		for (int32 Key : TimerKeys)
		{
			World->GetTimerManager().ClearTimer(ConditionTimerHandles[Key]);
		}
	}
	
	ConditionTimerHandles.Empty();
}

void AAO_PuzzleConditionChecker::ResetLinkedElements()
{
	if (!HasAuthority()) return;

	for (AActor* Actor : LinkedElements)
	{
		if (Actor)
		{
			if (IAO_Interface_PuzzleElement* PuzzleElement = Cast<IAO_Interface_PuzzleElement>(Actor))
			{
				PuzzleElement->ResetToInitialState();
			}
		}
	}
}

void AAO_PuzzleConditionChecker::DisableLinkedElements()
{
	if (!HasAuthority()) return;

	for (AActor* Actor : LinkedElements)
	{
		if (Actor)
		{
			if (IAO_Interface_PuzzleElement* PuzzleElement = Cast<IAO_Interface_PuzzleElement>(Actor))
			{
				PuzzleElement->SetInteractionEnabled(false);
			}
		}
	}
}

void AAO_PuzzleConditionChecker::EnableLinkedElements()
{
	if (!HasAuthority()) return;

	for (AActor* Actor : LinkedElements)
	{
		if (Actor)
		{
			if (IAO_Interface_PuzzleElement* PuzzleElement = Cast<IAO_Interface_PuzzleElement>(Actor))
			{
				PuzzleElement->SetInteractionEnabled(true);
			}
		}
	}
}

bool AAO_PuzzleConditionChecker::ShouldFailPuzzle() const
{
	// OR 조건, 필수 태그 다 받았는데 전부 실패여야 전체 실패
	if (!bRequireAllConditions)
	{
		int32 FailedConditionsWithAllTags = 0;
        
		for (int32 i = 0; i < Conditions.Num(); ++i)
		{
			const FPuzzleCondition& Condition = Conditions[i];
			bool bHasAllRequiredTags = false;
            
			if (Condition.bRequiresOrder && Condition.OrderedTags.Num() > 0)
			{
				bHasAllRequiredTags = true;
				for (const FGameplayTag& Tag : Condition.OrderedTags)
				{
					if (!ActiveEvents.HasTag(Tag))
					{
						bHasAllRequiredTags = false;
						break;
					}
				}
			}
			else if (Condition.RequiredTags.Num() > 0)
			{
				bHasAllRequiredTags = ActiveEvents.HasAll(Condition.RequiredTags);
			}
            
			if (bHasAllRequiredTags)
			{
				FailedConditionsWithAllTags++;
			}
		}
        
		// 모든 조건이 다 받았는데 실패면 전체 실패
		return FailedConditionsWithAllTags == Conditions.Num();
	}
	
	// AND 조건, 모든 태그를 다 받았는데 하나라도 조건 충족이 안되면 전체 실패
	for (int32 i = 0; i < Conditions.Num(); ++i)
	{
		const FPuzzleCondition& Condition = Conditions[i];

		// 순서 조건인 경우
		if (Condition.bRequiresOrder && Condition.OrderedTags.Num() > 0)
		{
			// 순서에 필요한 모든 태그가 들어왔는지 체크
			bool bAllOrderedTagsReceived = true;
			for (const FGameplayTag& Tag : Condition.OrderedTags)
			{
				if (!ActiveEvents.HasTag(Tag))
				{
					bAllOrderedTagsReceived = false;
					break;
				}
			}
            
			// 다 들어왔는데 순서가 틀림, 실패
			if (bAllOrderedTagsReceived)
			{
				return true;
			}
		}
		// 일반 조건인 경우
		else if (Condition.RequiredTags.Num() > 0)
		{
			// 필수 태그가 전부 들어왔는데 조건 실패
			// (BlockingTags 때문에 실패한 경우)
			if (ActiveEvents.HasAll(Condition.RequiredTags))
			{
				return true;
			}
		}
	}
    
	return false;
}