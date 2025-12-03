// HSJ : AO_DecayHoldElement.cpp
#include "Puzzle/Element/AO_DecayHoldElement.h"
#include "Net/UnrealNetwork.h"
#include "Puzzle/Actor/AO_PuzzleReactionActor.h"
#include "AbilitySystemComponent.h"
#include "AO_Log.h"
#include "Interaction/Component/AO_InteractionComponent.h"

AAO_DecayHoldElement::AAO_DecayHoldElement(const FObjectInitializer& ObjectInitializer)
    : Super(ObjectInitializer)
{
    ElementType = EPuzzleElementType::HoldToggle;
    bHandleToggleInOnInteractionSuccess = false; // Duration 도달 시 직접 처리
}

void AAO_DecayHoldElement::BeginPlay()
{
    Super::BeginPlay();
    
    if (HasAuthority())
    {
        if (PuzzleInteractionInfo.Duration <= 0.f)
        {
            AO_LOG(LogHSJ, Error, TEXT("[DecayHold] %s: Duration is 0!"), *GetName());
        }
        
        if (LinkedReactionActor && LinkedReactionActor->ReactionMode == EPuzzleReactionMode::Toggle)
        {
            if (!LinkedReactionActor->TriggerTag.IsValid())
            {
                AO_LOG(LogHSJ, Error, TEXT("[DecayHold] %s: Toggle mode but TriggerTag not set!"), *GetName());
            }
        }
    }
}

void AAO_DecayHoldElement::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
    StopProgressTimer();
    Super::EndPlay(EndPlayReason);
}

void AAO_DecayHoldElement::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);
    DOREPLIFETIME(AAO_DecayHoldElement, CurrentHoldProgress);
	DOREPLIFETIME(AAO_DecayHoldElement, ManualHoldActors);
}

bool AAO_DecayHoldElement::CanInteraction(const FAO_InteractionQuery& InteractionQuery) const
{
    if (!Super::CanInteraction(InteractionQuery))
        return false;

	// 다른 플레이어가 홀드 중이면 차단 (본인은 허용)
    if (IsAnyoneHolding())
    {
        AActor* RequestingActor = InteractionQuery.RequestingAvatar.Get();
        if (!RequestingActor)
            return false;

    	// 본인이면 허용
        for (const TWeakObjectPtr<AActor>& Interactor : CachedInteractors)
        {
            if (Interactor.Get() == RequestingActor)
            {
                return true;
            }
        }
        
        for (const TWeakObjectPtr<AActor>& Interactor : ManualHoldActors)
        {
            if (Interactor.Get() == RequestingActor)
            {
                return true;
            }
        }
        
        return false;
    }
    
    return true;
}

void AAO_DecayHoldElement::OnInteractActiveStarted(AActor* Interactor)
{
    Super::OnInteractActiveStarted(Interactor);
    
    if (!HasAuthority()) return;

	// Duration 도달 후 실제 키 입력 상태 추적을 위한 백업 리스트
    ManualHoldActors.AddUnique(Interactor);

	StartProgressTimer();
}

void AAO_DecayHoldElement::OnInteractActiveEnded(AActor* Interactor)
{
    if (!HasAuthority())
    {
        Super::OnInteractActiveEnded(Interactor);
        return;
    }

    const float TargetDuration = PuzzleInteractionInfo.Duration;

	// HoldActive인 경우 Duration 도달 후에도 키를 누르고 있으면 홀드 유지
	if (LinkedReactionActor && LinkedReactionActor->ReactionMode == EPuzzleReactionMode::HoldActive)
	{
		if (CurrentHoldProgress >= TargetDuration - 0.15f)
		{
			if (UAO_InteractionComponent* InteractionComp = Interactor->FindComponentByClass<UAO_InteractionComponent>())
			{
				if (InteractionComp->bIsHoldingInteract)
				{
					// 여전히 키를 누르고 있음, Super 호출 스킵하여 CachedInteractors 유지
					return;
				}
			}
		}
	}
	// Toggle인 경우 UpdateProgress에서 처리 못 한 경우 백업 처리
	else if (LinkedReactionActor && LinkedReactionActor->ReactionMode == EPuzzleReactionMode::Toggle)
	{
		// 토글 성공
		if (CurrentHoldProgress >= TargetDuration- 0.15f)
		{
			CurrentHoldProgress = TargetDuration;
			HandleToggleCompletion();
		}
		else
		{
			// 중간에 뗀 경우, 토글 실패, 진행도 리셋
			CurrentHoldProgress = 0.0f;
		}
	}
    
    Super::OnInteractActiveEnded(Interactor);
    ManualHoldActors.RemoveSingleSwap(Interactor);
}

void AAO_DecayHoldElement::ResetToInitialState()
{
    Super::ResetToInitialState();
    CurrentHoldProgress = 0.0f;
    ManualHoldActors.Empty();
}

bool AAO_DecayHoldElement::IsAnyoneHolding() const
{
    for (const TWeakObjectPtr<AActor>& Interactor : CachedInteractors)
    {
        if (Interactor.IsValid())
        {
            return true;
        }
    }
    for (const TWeakObjectPtr<AActor>& Interactor : ManualHoldActors)
    {
        if (Interactor.IsValid())
        {
            return true;
        }
    }
    
    return false;
}

void AAO_DecayHoldElement::StartProgressTimer()
{
    if (ProgressTimerHandle.IsValid()) return;
    
	TWeakObjectPtr<AAO_DecayHoldElement> WeakThis(this);
	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().SetTimer(
			ProgressTimerHandle,
			FTimerDelegate::CreateWeakLambda(this, [WeakThis]()
			{
				if (AAO_DecayHoldElement* StrongThis = WeakThis.Get())
				{
					StrongThis->UpdateProgress();
				}
			}),
			0.1f,
			true
		);
	}
}

void AAO_DecayHoldElement::StopProgressTimer()
{
    GetWorldTimerManager().ClearTimer(ProgressTimerHandle);
}

void AAO_DecayHoldElement::UpdateProgress()
{
    if (!HasAuthority()) return;

    const float DeltaTime = 0.1f;
    const float TargetDuration = PuzzleInteractionInfo.Duration;
    
    if (TargetDuration <= 0.f) return;

    bool bHolding = IsAnyoneHolding();

	// 진행도 증가/감소
    if (bHolding)
    {
        CurrentHoldProgress += DeltaTime;
        
        if (CurrentHoldProgress >= TargetDuration - 0.05f)
        {
            CurrentHoldProgress = TargetDuration;

        	// Toggle인 경우 즉시 토글 처리
        	if (LinkedReactionActor && LinkedReactionActor->ReactionMode == EPuzzleReactionMode::Toggle)
        	{
        		HandleToggleCompletion();
        		ManualHoldActors.Empty();
        		CachedInteractors.Empty();
        		StopProgressTimer();
        		return;
        	}
        }
    }
    else
    {
    	// HoldActive면 진행도 감소
    	if (LinkedReactionActor && LinkedReactionActor->ReactionMode == EPuzzleReactionMode::HoldActive)
    	{
    		CurrentHoldProgress -= DecayRate * DeltaTime;
            
    		if (CurrentHoldProgress <= 0.0f)
    		{
    			CurrentHoldProgress = 0.0f;
    			ManualHoldActors.Empty();
    			CachedInteractors.Empty();
    		}
    	}
    }

	// HoldActive인 경우 Duration 도달 후 실제 키 입력 상태 확인
	if (bHolding && CurrentHoldProgress >= TargetDuration - 0.05f)
	{
		if (LinkedReactionActor && LinkedReactionActor->ReactionMode == EPuzzleReactionMode::HoldActive)
		{
			bool bStillHoldingKey = false;

			// ManualHoldActors에서 실제 키 입력 확인
			for (const TWeakObjectPtr<AActor>& Actor : ManualHoldActors)
			{
				if (Actor.IsValid())
				{
					if (UAO_InteractionComponent* InteractionComp = Actor->FindComponentByClass<UAO_InteractionComponent>())
					{
						if (InteractionComp->bIsHoldingInteract)
						{
							bStillHoldingKey = true;
							break;
						}
					}
				}
			}

			// 키를 놓았으면 양쪽 리스트 모두 정리
			if (!bStillHoldingKey)
			{
				ManualHoldActors.Empty();
				CachedInteractors.Empty();
				bHolding = false;
			}
		}
	}

	// 타이머 정지 조건, 아무도 홀드 안 하고 진행도 0
	if (!bHolding && CurrentHoldProgress <= 0.0f)
	{
		StopProgressTimer();
		return;
	}
	
	// HoldActive인 경우 진행도를 ReactionActor에 진행도 실시간 전달
    if (LinkedReactionActor && LinkedReactionActor->ReactionMode == EPuzzleReactionMode::HoldActive)
    {
        float Progress = TargetDuration > 0.f ? CurrentHoldProgress / TargetDuration : 0.f;
        LinkedReactionActor->SetProgress(Progress);
    }
}

void AAO_DecayHoldElement::HandleToggleCompletion()
{
    if (!LinkedReactionActor) return;
    if (LinkedReactionActor->ReactionMode != EPuzzleReactionMode::Toggle) return;

    FGameplayTag TriggerTag = LinkedReactionActor->TriggerTag;
    if (!TriggerTag.IsValid()) return;

    UAbilitySystemComponent* ASC = LinkedReactionActor->GetAbilitySystemComponent();
    if (!ASC) return;

	// 상태 토글 및 태그 추가/제거
    bIsActivated = !bIsActivated;
    
    if (bIsActivated)
    {
        ASC->AddLooseGameplayTag(TriggerTag);
    }
    else
    {
        ASC->RemoveLooseGameplayTag(TriggerTag);
    }
    
    OnRep_IsActivated();
    CurrentHoldProgress = 0.0f;
}