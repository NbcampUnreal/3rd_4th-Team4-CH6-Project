// HSJ : AO_DecayHoldElement.cpp
#include "Puzzle/Element/AO_DecayHoldElement.h"
#include "Net/UnrealNetwork.h"

AAO_DecayHoldElement::AAO_DecayHoldElement(const FObjectInitializer& ObjectInitializer)
    : Super(ObjectInitializer)
{
	// 베이스 타입은 HoldToggle 기반
    ElementType = EPuzzleElementType::HoldToggle;  
}

void AAO_DecayHoldElement::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);
    DOREPLIFETIME(AAO_DecayHoldElement, CurrentHoldProgress);
}

void AAO_DecayHoldElement::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
    StopProgressTimer();
    Super::EndPlay(EndPlayReason);
}

void AAO_DecayHoldElement::OnInteractActiveStarted(AActor* Interactor)
{
	Super::OnInteractActiveStarted(Interactor);
    
    if (!HasAuthority()) return;

    CurrentHolder = Interactor;
    StartProgressTimer();
}

void AAO_DecayHoldElement::OnInteractActiveEnded(AActor* Interactor)
{
	Super::OnInteractActiveEnded(Interactor);
    
    if (!HasAuthority()) return;

    if (CurrentHolder == Interactor)
    {
        CurrentHolder = nullptr;
        // 타이머는 계속 돌면서 감소 처리
    }
}

void AAO_DecayHoldElement::ResetToInitialState()
{
    Super::ResetToInitialState();
    CurrentHoldProgress = 0.0f;
    StopProgressTimer();
}

void AAO_DecayHoldElement::StartProgressTimer()
{
    if (HoldProgressTimerHandle.IsValid()) return;
    
    FTimerDelegate TimerDelegate;
    TimerDelegate.BindWeakLambda(this, [this]()
    {
        UpdateHoldProgress();
    });
    
    GetWorldTimerManager().SetTimer(
        HoldProgressTimerHandle,
        TimerDelegate,
        0.1f,
        true
    );
}

void AAO_DecayHoldElement::StopProgressTimer()
{
    GetWorldTimerManager().ClearTimer(HoldProgressTimerHandle);
}

void AAO_DecayHoldElement::UpdateHoldProgress()
{
    if (!HasAuthority()) return;

    const float DeltaTime = 0.1f;

    if (CurrentHolder)
    {
        CurrentHoldProgress += DeltaTime;
        
        if (CurrentHoldProgress >= MaxHoldTime)
        {
            CurrentHoldProgress = MaxHoldTime;
            bIsActivated = true;
            BroadcastPuzzleEvent(true);
            StopProgressTimer();
            OnRep_IsActivated();
        }
    }
    else
    {
        CurrentHoldProgress -= DecayRate * DeltaTime;
        
        if (CurrentHoldProgress <= 0.0f)
        {
            CurrentHoldProgress = 0.0f;
            StopProgressTimer();
        }
    }
}