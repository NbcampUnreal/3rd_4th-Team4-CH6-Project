// HSJ : AO_RotationPuzzleElement.cpp
#include "Puzzle/Element/AO_RotationPuzzleElement.h"
#include "Net/UnrealNetwork.h"
#include "Puzzle/Checker/AO_PuzzleConditionChecker.h"
#include "Components/StaticMeshComponent.h"
#include "AO_Log.h"

AAO_RotationPuzzleElement::AAO_RotationPuzzleElement(const FObjectInitializer& ObjectInitializer)
    : Super(ObjectInitializer)
{
    ElementType = EPuzzleElementType::Toggle;
}

void AAO_RotationPuzzleElement::BeginPlay()
{
    Super::BeginPlay();

    // 초기 회전값 저장 (원위치 계산용)
    if (MeshComponent)
    {
        InitialRotation = MeshComponent->GetRelativeRotation();
    }
}

void AAO_RotationPuzzleElement::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
    GetWorldTimerManager().ClearTimer(RotationTimerHandle);
    Super::EndPlay(EndPlayReason);
}

void AAO_RotationPuzzleElement::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);
    
    DOREPLIFETIME(AAO_RotationPuzzleElement, CurrentRotationIndex);
	DOREPLIFETIME(AAO_RotationPuzzleElement, bIsRotating);
}

void AAO_RotationPuzzleElement::OnInteractionSuccess(AActor* Interactor)
{
    if (!HasAuthority()) return;
	if (bIsRotating) return;

    RotateToNext();
}

void AAO_RotationPuzzleElement::OnInspectionMeshClicked(UPrimitiveComponent* ClickedComponent)
{
    if (!HasAuthority()) return;
	if (bIsRotating) return;

    RotateToNext();
}

void AAO_RotationPuzzleElement::ResetToInitialState()
{
    Super::ResetToInitialState();

    CurrentRotationIndex = 0;
	bIsRotating = false;
    
    GetWorldTimerManager().ClearTimer(RotationTimerHandle);

    if (MeshComponent)
    {
        MeshComponent->SetRelativeRotation(InitialRotation);
    }
}

void AAO_RotationPuzzleElement::RotateToNext()
{
    if (!HasAuthority()) return;

	bIsRotating = true;

    // 다음 회전 인덱스로 이동
    CurrentRotationIndex = (CurrentRotationIndex + 1) % NumRotationSteps;

    // 정답 체크 및 태그 전송
    CheckCorrectRotation();

    if (CurrentRotationIndex == 0)
    {
        // 한 사이클 완료 시 정확히 원위치로
        TargetRotation = InitialRotation;
    }
    else
    {
        // 각 단계별 회전 각도 계산
        float AnglePerStep = 360.0f / NumRotationSteps;
        FRotator StepRotation = RotationAxisPerStep;
        StepRotation.Normalize();
        
        // 각 축별로 회전량 계산
        TargetRotation = InitialRotation + FRotator(
            StepRotation.Pitch * AnglePerStep * CurrentRotationIndex / 90.0f,
            StepRotation.Yaw * AnglePerStep * CurrentRotationIndex / 90.0f,
            StepRotation.Roll * AnglePerStep * CurrentRotationIndex / 90.0f
        );
    }

    TWeakObjectPtr<AAO_RotationPuzzleElement> WeakThis(this);
    
    GetWorldTimerManager().SetTimer(
        RotationTimerHandle,
        FTimerDelegate::CreateWeakLambda(this, [WeakThis]()
        {
            if (AAO_RotationPuzzleElement* StrongThis = WeakThis.Get())
            {
                StrongThis->UpdateRotationAnimation();
            }
        }),
        0.016f,
        true
    );
}

void AAO_RotationPuzzleElement::CheckCorrectRotation()
{
    if (!LinkedChecker) return;

    bool bWasCorrect = bIsActivated;
    bool bIsCorrect = (CurrentRotationIndex == CorrectRotationIndex);

    // 상태 변화 없으면 스킵
    if (bWasCorrect == bIsCorrect) return;

    if (bIsCorrect)
    {
        // 정답이면 성공 태그 추가, 실패 태그 제거
        bIsActivated = true;
        if (ActivatedEventTag.IsValid())
        {
            LinkedChecker->OnPuzzleEvent(ActivatedEventTag, GetInstigator());
        }
        if (DeactivatedEventTag.IsValid())
        {
            LinkedChecker->RemovePuzzleEvent(DeactivatedEventTag);
        }
    }
    else
    {
        // 오답이면 성공 태그 제거, 실패 태그 추가
        bIsActivated = false;
        if (ActivatedEventTag.IsValid())
        {
            LinkedChecker->RemovePuzzleEvent(ActivatedEventTag);
        }
        if (DeactivatedEventTag.IsValid())
        {
            LinkedChecker->OnPuzzleEvent(DeactivatedEventTag, GetInstigator());
        }
    }
}

void AAO_RotationPuzzleElement::UpdateRotationAnimation()
{
    if (!MeshComponent) return;

    // 현재 회전에서 목표 회전으로 보간
    FRotator CurrentRotation = MeshComponent->GetRelativeRotation();
    FRotator NewRotation = FMath::RInterpTo(CurrentRotation, TargetRotation, 0.016f, RotationSpeed);

    MeshComponent->SetRelativeRotation(NewRotation);

    // 목표 회전에 도달했는지 체크
    bool bReachedTarget = IsRotatorNearlyEqual(NewRotation, TargetRotation, 0.5f);

    if (bReachedTarget)
    {
        // 정확히 목표값으로 설정 (오차 제거)
        MeshComponent->SetRelativeRotation(TargetRotation);
        
        GetWorldTimerManager().ClearTimer(RotationTimerHandle);

    	if (HasAuthority())
    	{
    		bIsRotating = false;
    	}
    }
}

void AAO_RotationPuzzleElement::OnRep_CurrentRotationIndex()
{
    // 클라이언트에서 복제 받았을 때 회전 애니메이션 재생
    if (MeshComponent)
    {
        if (CurrentRotationIndex == 0)
        {
            TargetRotation = InitialRotation;
        }
        else
        {
            float AnglePerStep = 360.0f / NumRotationSteps;
            FRotator StepRotation = RotationAxisPerStep;
            StepRotation.Normalize();
            
            TargetRotation = InitialRotation + FRotator(
                StepRotation.Pitch * AnglePerStep * CurrentRotationIndex / 90.0f,
                StepRotation.Yaw * AnglePerStep * CurrentRotationIndex / 90.0f,
                StepRotation.Roll * AnglePerStep * CurrentRotationIndex / 90.0f
            );
        }

        TWeakObjectPtr<AAO_RotationPuzzleElement> WeakThis(this);
        
        GetWorldTimerManager().SetTimer(
            RotationTimerHandle,
            FTimerDelegate::CreateWeakLambda(this, [WeakThis]()
            {
                if (AAO_RotationPuzzleElement* StrongThis = WeakThis.Get())
                {
                    StrongThis->UpdateRotationAnimation();
                }
            }),
            0.016f,
            true
        );
    }
}

bool AAO_RotationPuzzleElement::IsRotatorNearlyEqual(const FRotator& A, const FRotator& B, float Tolerance) const
{
    // 각 축별로 정규화된 각도 차이가 허용 오차 내에 있는지 체크
    return FMath::Abs(FRotator::NormalizeAxis(A.Pitch - B.Pitch)) <= Tolerance &&
           FMath::Abs(FRotator::NormalizeAxis(A.Yaw - B.Yaw)) <= Tolerance &&
           FMath::Abs(FRotator::NormalizeAxis(A.Roll - B.Roll)) <= Tolerance;
}