// HSJ : AO_PuzzleReactionActor.cpp
#include "Puzzle/Actor/AO_PuzzleReactionActor.h"
#include "AbilitySystemComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Net/UnrealNetwork.h"
#include "AO_Log.h"

AAO_PuzzleReactionActor::AAO_PuzzleReactionActor()
{
    bReplicates = true;

    RootComp = CreateDefaultSubobject<USceneComponent>(TEXT("RootComp"));
    RootComponent = RootComp;

    MeshComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("MeshComponent"));
    MeshComponent->SetupAttachment(RootComponent);

    AbilitySystemComponent = CreateDefaultSubobject<UAbilitySystemComponent>(TEXT("AbilitySystemComponent"));
    AbilitySystemComponent->SetIsReplicated(true);
    AbilitySystemComponent->SetReplicationMode(EGameplayEffectReplicationMode::Minimal);
}

UAbilitySystemComponent* AAO_PuzzleReactionActor::GetAbilitySystemComponent() const
{
    return AbilitySystemComponent;
}

void AAO_PuzzleReactionActor::BeginPlay()
{
    Super::BeginPlay();

    if (MeshComponent)
    {
        InitialLocation = MeshComponent->GetRelativeLocation();
        InitialRotation = MeshComponent->GetRelativeRotation();
    }

    if (HasAuthority() && AbilitySystemComponent)
    {
        AbilitySystemComponent->InitAbilityActorInfo(this, this);

        // TriggerTag 변경 감지 등록
        if (TriggerTag.IsValid())
        {
            AbilitySystemComponent->RegisterGameplayTagEvent(
                TriggerTag,
                EGameplayTagEventType::NewOrRemoved
            ).AddUObject(this, &AAO_PuzzleReactionActor::OnTriggerTagChanged);
        }
    }
}

void AAO_PuzzleReactionActor::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
    GetWorldTimerManager().ClearTimer(TransformTimerHandle);
    Super::EndPlay(EndPlayReason);
}

void AAO_PuzzleReactionActor::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);
    
    DOREPLIFETIME(AAO_PuzzleReactionActor, bIsActivated);
    DOREPLIFETIME(AAO_PuzzleReactionActor, TargetProgress);
}

void AAO_PuzzleReactionActor::SetProgress(float Progress)
{
    if (!HasAuthority()) return;
    if (ReactionMode != EPuzzleReactionMode::HoldActive) return;

    // 진행도 클램프 (0.0 ~ 1.0)
    TargetProgress = FMath::Clamp(Progress, 0.0f, 1.0f);

    // Transform 타이머가 없으면 시작
    if (!GetWorldTimerManager().IsTimerActive(TransformTimerHandle))
    {
        TWeakObjectPtr<AAO_PuzzleReactionActor> WeakThis(this);
        GetWorldTimerManager().SetTimer(
            TransformTimerHandle,
            FTimerDelegate::CreateWeakLambda(this, [WeakThis]()
            {
                if (AAO_PuzzleReactionActor* StrongThis = WeakThis.Get())
                {
                    StrongThis->UpdateTransform();
                }
            }),
            0.016f,
            true
        );
    }
}

void AAO_PuzzleReactionActor::OnTriggerTagChanged(const FGameplayTag Tag, int32 NewCount)
{
	if (!HasAuthority()) return;

	if (ReactionMode == EPuzzleReactionMode::HoldActive) return;

	if (NewCount > 0)
	{
		ActivateReaction();
	}
	else
	{
		DeactivateReaction();
	}
}

void AAO_PuzzleReactionActor::ActivateReaction()
{
    if (!HasAuthority()) return;

    switch (ReactionMode)
    {
    case EPuzzleReactionMode::OneTime:
        if (!bIsActivated)
        {
            bIsActivated = true;
        }
        break;

    case EPuzzleReactionMode::Toggle:
        bIsActivated = true;
        break;

    case EPuzzleReactionMode::HoldActive:
        // SetProgress()로 제어
        return;
    }

    // Transform 애니메이션 시작
    TWeakObjectPtr<AAO_PuzzleReactionActor> WeakThis(this);
    GetWorldTimerManager().SetTimer(
        TransformTimerHandle,
        FTimerDelegate::CreateWeakLambda(this, [WeakThis]()
        {
            if (AAO_PuzzleReactionActor* StrongThis = WeakThis.Get())
            {
                StrongThis->UpdateTransform();
            }
        }),
        0.016f,
        true
    );
}

void AAO_PuzzleReactionActor::DeactivateReaction()
{
    if (!HasAuthority()) return;

    if (ReactionMode == EPuzzleReactionMode::OneTime) return;

    // Toggle은 Tag 제거 시 비활성화
    if (ReactionMode == EPuzzleReactionMode::Toggle)
    {
        bIsActivated = false;
        
        // Transform 애니메이션 시작
        TWeakObjectPtr<AAO_PuzzleReactionActor> WeakThis(this);
        GetWorldTimerManager().SetTimer(
            TransformTimerHandle,
            FTimerDelegate::CreateWeakLambda(this, [WeakThis]()
            {
                if (AAO_PuzzleReactionActor* StrongThis = WeakThis.Get())
                {
                    StrongThis->UpdateTransform();
                }
            }),
            0.016f,
            true
        );
    }
}

void AAO_PuzzleReactionActor::UpdateTransform()
{
    if (!MeshComponent) return;

    FVector CurrentLocation = MeshComponent->GetRelativeLocation();
    FRotator CurrentRotation = MeshComponent->GetRelativeRotation();

    float CurrentProgress = 0.0f;
    if (ReactionMode == EPuzzleReactionMode::HoldActive)
    {
        // HoldActive는 SetProgress()로 설정된 진행도 사용
        CurrentProgress = TargetProgress;
    }
    else
    {
        // OneTime/Toggle은 활성화 상태에 따라 0 또는 1
        CurrentProgress = bIsActivated ? 1.0f : 0.0f;
    }

    FVector DesiredLocation = FMath::Lerp(InitialLocation, bUseLocation ? TargetRelativeLocation : InitialLocation, CurrentProgress);
    FRotator DesiredRotation = FMath::Lerp(InitialRotation, bUseRotation ? TargetRelativeRotation : InitialRotation, CurrentProgress);

    FVector NewLocation = FMath::VInterpTo(CurrentLocation, DesiredLocation, 0.016f, TransformSpeed);
    FRotator NewRotation = FMath::RInterpTo(CurrentRotation, DesiredRotation, 0.016f, TransformSpeed);

    MeshComponent->SetRelativeLocation(NewLocation);
    MeshComponent->SetRelativeRotation(NewRotation);

    // 목표에 도달했는지 체크
    bool bReachedTarget = FVector::Dist(NewLocation, DesiredLocation) < 0.1f && 
                          IsRotatorNearlyEqual(NewRotation, DesiredRotation, 0.5f);

    // HoldActive가 아니면서 목표 도달 시 타이머 정지
    if (bReachedTarget && ReactionMode != EPuzzleReactionMode::HoldActive)
    {
        GetWorldTimerManager().ClearTimer(TransformTimerHandle);
    }
}

void AAO_PuzzleReactionActor::OnRep_IsActivated()
{
    // 클라이언트 Transform 애니메이션 시작
    if (!HasAuthority())
    {
        TWeakObjectPtr<AAO_PuzzleReactionActor> WeakThis(this);
        GetWorldTimerManager().SetTimer(
            TransformTimerHandle,
            FTimerDelegate::CreateWeakLambda(this, [WeakThis]()
            {
                if (AAO_PuzzleReactionActor* StrongThis = WeakThis.Get())
                {
                    StrongThis->UpdateTransform();
                }
            }),
            0.016f,
            true
        );
    }
}

void AAO_PuzzleReactionActor::OnRep_TargetProgress()
{
    // 클라이언트 HoldActive 진행도 변경 시 Transform 업데이트
    if (!HasAuthority() && ReactionMode == EPuzzleReactionMode::HoldActive)
    {
        if (!GetWorldTimerManager().IsTimerActive(TransformTimerHandle))
        {
            TWeakObjectPtr<AAO_PuzzleReactionActor> WeakThis(this);
            GetWorldTimerManager().SetTimer(
                TransformTimerHandle,
                FTimerDelegate::CreateWeakLambda(this, [WeakThis]()
                {
                    if (AAO_PuzzleReactionActor* StrongThis = WeakThis.Get())
                    {
                        StrongThis->UpdateTransform();
                    }
                }),
                0.016f,
                true
            );
        }
    }
}

bool AAO_PuzzleReactionActor::IsRotatorNearlyEqual(const FRotator& A, const FRotator& B, float Tolerance) const
{
    return FMath::Abs(FRotator::NormalizeAxis(A.Pitch - B.Pitch)) <= Tolerance &&
           FMath::Abs(FRotator::NormalizeAxis(A.Yaw - B.Yaw)) <= Tolerance &&
           FMath::Abs(FRotator::NormalizeAxis(A.Roll - B.Roll)) <= Tolerance;
}