// HSJ : AO_DecayHoldElement.cpp
#include "Puzzle/Element/AO_DecayHoldElement.h"
#include "Net/UnrealNetwork.h"
#include "Puzzle/Actor/AO_PuzzleReactionActor.h"
#include "AO_Log.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Interaction/Component/AO_InteractionComponent.h"

AAO_DecayHoldElement::AAO_DecayHoldElement(const FObjectInitializer& ObjectInitializer)
    : Super(ObjectInitializer)
{
    ElementType = EPuzzleElementType::HoldToggle;
}

void AAO_DecayHoldElement::BeginPlay()
{
    Super::BeginPlay();
    
    if (HasAuthority())
    {
        if (!LinkedReactionActor)
        {
            AO_LOG(LogHSJ, Error, TEXT("[DecayHold] %s: LinkedReactionActor not set!"), *GetName());
        }
        else if (LinkedReactionActor->ReactionMode != EPuzzleReactionMode::HoldActive)
        {
            AO_LOG(LogHSJ, Warning, TEXT("[DecayHold] %s: LinkedReactionActor should use HoldActive mode"), *GetName());
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

    NotifyCount = 0;
    bIsLeverUp = false;

	// Duration 도달 후 실제 키 입력 상태 추적을 위한 백업 리스트
    ManualHoldActors.AddUnique(Interactor);
    StartProgressTimer();
    PlayStartMontage();
}

void AAO_DecayHoldElement::OnInteractActiveEnded(AActor* Interactor)
{
	Super::OnInteractActiveEnded(Interactor);
    
	if (!HasAuthority())
	{
		return;
	}

	ManualHoldActors.RemoveSingleSwap(Interactor);
    
	// 비정상 종료 처리인 경우
	if (NotifyCount > 0 && NotifyCount < 3)
	{
		NotifyCount = 0;
        
		if (bIsLeverUp)
		{
			bIsLeverUp = false;
			MulticastLeverAction(false);
		}
        
		MulticastSetMovementForActor(Interactor, true);
	}
}

void AAO_DecayHoldElement::ResetToInitialState()
{
    Super::ResetToInitialState();
    CurrentHoldProgress = 0.0f;
    ManualHoldActors.Empty();
    bIsLeverUp = false;
    NotifyCount = 0;
}

void AAO_DecayHoldElement::OnNotifyReceived()
{
	if (!HasAuthority()) return;
    
	NotifyCount++;
    
	// 상호작용 Actor 찾기
	AActor* InteractingActor = nullptr;
	if (CachedInteractors.Num() > 0)
	{
		InteractingActor = CachedInteractors[0].Get();
	}
    
	if (!InteractingActor)
	{
		AO_LOG(LogHSJ, Error, TEXT("[DecayHold] No interacting actor!"));
		return;
	}
    
	switch (NotifyCount)
	{
	case 1:
		bIsLeverUp = true;
		MulticastLeverAction(true);
		break;
        
	case 2:
		MulticastMontageControl(InteractingActor, true, 0.0f);
		MulticastSetMovementForActor(InteractingActor, false);
		break;
        
	case 3:
		bIsLeverUp = false;
		MulticastLeverAction(false);
		break;
	}
}

void AAO_DecayHoldElement::StopEarly()
{
	if (!HasAuthority()) return;
    
	AActor* InteractingActor = nullptr;
	if (CachedInteractors.Num() > 0)
	{
		InteractingActor = CachedInteractors[0].Get();
	}
    
	if (InteractingActor)
	{
		MulticastMontageControl(InteractingActor, false, 0.0f);
	}
    
	StopProgressTimer();
    
	ManualHoldActors.Empty();
	CachedInteractors.Empty();
    
	StartProgressTimer();
    
	NotifyCount = 0;
    
	if (bIsLeverUp)
	{
		bIsLeverUp = false;
		MulticastLeverAction(false);
	}
}

void AAO_DecayHoldElement::ReleasePause()
{
	if (!HasAuthority()) return;
    
	ManualHoldActors.Empty();
    
	AActor* InteractingActor = nullptr;
	if (CachedInteractors.Num() > 0)
	{
		InteractingActor = CachedInteractors[0].Get();
	}
    
	if (InteractingActor)
	{
		MulticastMontageControl(InteractingActor, true, 1.0f);
		MulticastSetMovementForActor(InteractingActor, true);
	}
}

void AAO_DecayHoldElement::CleanupAfterMontage()
{
	if (!HasAuthority()) return;
    
	AActor* InteractingActor = nullptr;
	if (CachedInteractors.Num() > 0)
	{
		InteractingActor = CachedInteractors[0].Get();
	}
    
	CachedInteractors.Empty();
    
	if (InteractingActor)
	{
		MulticastSetMovementForActor(InteractingActor, true);
	}
}

float AAO_DecayHoldElement::GetActiveMontageRemainingTime() const
{
    const FAO_InteractionInfo& Info = PuzzleInteractionInfo;
    if (!Info.ActiveMontage) return 0.5f;

    for (const TWeakObjectPtr<AActor>& Actor : CachedInteractors)
    {
        if (AActor* ValidActor = Actor.Get())
        {
            if (ACharacter* Character = Cast<ACharacter>(ValidActor))
            {
                if (UAnimInstance* AnimInstance = Character->GetMesh()->GetAnimInstance())
                {
                    if (AnimInstance->Montage_IsPlaying(Info.ActiveMontage))
                    {
                        float Remaining = Info.ActiveMontage->GetPlayLength() - AnimInstance->Montage_GetPosition(Info.ActiveMontage);
                        return FMath::Max(Remaining, 0.2f) + 0.2f;
                    }
                }
            }
        }
    }
    
    return 0.5f;
}

bool AAO_DecayHoldElement::IsAnyoneHolding() const
{
	for (const TWeakObjectPtr<AActor>& Interactor : ManualHoldActors)
	{
		if (Interactor.IsValid())
			return true;
	}
	return false;
}

void AAO_DecayHoldElement::StartProgressTimer()
{
    if (ProgressTimerHandle.IsValid()) return;
    
	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().SetTimer(
			ProgressTimerHandle,
			FTimerDelegate::CreateWeakLambda(this, [this]()
			{
				UpdateProgress();
			}),
			0.1f,
			true
		);
	}
}

void AAO_DecayHoldElement::StopProgressTimer()
{
	UWorld* World = GetWorld();
	if (World)
	{
		World->GetTimerManager().ClearTimer(ProgressTimerHandle);
	}
}

void AAO_DecayHoldElement::UpdateProgress()
{
	checkf(HasAuthority(), TEXT("UpdateProgress called on client"));

    const float DeltaTime = 0.1f;
    const float TargetDuration = PuzzleInteractionInfo.Duration;
    if (TargetDuration <= 0.f) return;

    bool bHolding = IsAnyoneHolding();

    if (bHolding)
    {
        CurrentHoldProgress = FMath::Min(CurrentHoldProgress + DeltaTime, TargetDuration);
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
    
    if (LinkedReactionActor)
    {
        LinkedReactionActor->SetProgress(CurrentHoldProgress / TargetDuration);
    }
}

void AAO_DecayHoldElement::PlayStartMontage()
{
    const FAO_InteractionInfo& Info = PuzzleInteractionInfo;
    if (!Info.ActiveMontage) return;

    for (const TWeakObjectPtr<AActor>& Actor : CachedInteractors)
    {
        if (AActor* ValidActor = Actor.Get())
        {
            if (UAO_InteractionComponent* InteractionComp = ValidActor->FindComponentByClass<UAO_InteractionComponent>())
            {
                InteractionComp->MulticastPlayInteractionMontage(
                    Info.ActiveMontage,
                    GetInteractionTransform(),
                    WarpTargetName
                );
            }
        }
    }
}

void AAO_DecayHoldElement::MulticastLeverAction_Implementation(bool bActivate)
{
    StartInteractionAnimation(bActivate);
    MulticastPlayInteractionSound();
}

void AAO_DecayHoldElement::MulticastMontageControl_Implementation(AActor* TargetActor, bool bPlay, float PlayRate)
{
    if (!TargetActor)
    {
        AO_LOG(LogHSJ, Error, TEXT("[DecayHold] MulticastMontageControl: TargetActor is null!"));
        return;
    }
    
    const FAO_InteractionInfo& Info = PuzzleInteractionInfo;
    if (!Info.ActiveMontage) return;

    if (ACharacter* Character = Cast<ACharacter>(TargetActor))
    {
        if (UAnimInstance* AnimInstance = Character->GetMesh()->GetAnimInstance())
        {
            if (bPlay && AnimInstance->Montage_IsPlaying(Info.ActiveMontage))
            {
                AnimInstance->Montage_SetPlayRate(Info.ActiveMontage, PlayRate);
            }
            else if (!bPlay)
            {
                AnimInstance->Montage_Stop(0.2f, Info.ActiveMontage);
            }
        }
    }
}

void AAO_DecayHoldElement::MulticastSetMovementForActor_Implementation(AActor* TargetActor, bool bEnable)
{
    if (!TargetActor)
    {
        AO_LOG(LogHSJ, Error, TEXT("[DecayHold] MulticastSetMovementForActor: TargetActor is null!"));
        return;
    }
    
    if (ACharacter* Character = Cast<ACharacter>(TargetActor))
    {
        if (UCharacterMovementComponent* Movement = Character->GetCharacterMovement())
        {
            if (bEnable)
            {
                Movement->SetMovementMode(MOVE_Walking);
            }
            else
            {
                Movement->DisableMovement();
            }
        }
    }
}