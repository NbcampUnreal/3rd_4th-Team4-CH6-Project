// HSJ : AO_RotationPuzzleElement.h
#pragma once

#include "CoreMinimal.h"
#include "Puzzle/Element/AO_PuzzleElement.h"
#include "Interaction/Interface/AO_Interface_Inspectable.h"
#include "AO_RotationPuzzleElement.generated.h"

/**
 * 회전 퍼즐 요소
 * - 상호작용하거나 Inspection에서 클릭 시 N도씩 회전
 * - 설정된 방향일 때만 성공 태그 전송
 * - NumRotationSteps로 회전 단계 수 설정 (4방향, 6방향 등)
 */
UCLASS()
class AO_API AAO_RotationPuzzleElement : public AAO_PuzzleElement, public IAO_Interface_Inspectable
{
    GENERATED_BODY()

public:
    AAO_RotationPuzzleElement(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

    virtual void OnInteractionSuccess(AActor* Interactor) override;
    virtual void ResetToInitialState() override;
    virtual void OnInspectionMeshClicked(UPrimitiveComponent* ClickedComponent) override;

protected:
    virtual void BeginPlay() override;
    virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
    virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

    UFUNCTION()
    void OnRep_CurrentRotationIndex();

private:
    void RotateToNext();
    void CheckCorrectRotation();
    void UpdateRotationAnimation();
    
    // 두 Rotator가 거의 같은지 비교 (오차 허용)
    bool IsRotatorNearlyEqual(const FRotator& A, const FRotator& B, float Tolerance = 0.5f) const;

public:
    // 회전 단계 수 (4 = 90도씩 4번, 6 = 60도씩 6번)
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Rotation", meta=(ClampMin="2", ClampMax="12"))
    int32 NumRotationSteps = 4;

    // 정답 회전 인덱스
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Rotation")
    int32 CorrectRotationIndex = 0;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Rotation|Animation", meta=(ClampMin="0.1"))
    float RotationSpeed = 3.0f;

    // 한 단계당 회전 방향
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Rotation|Animation")
    FRotator RotationAxisPerStep = FRotator(0, 90, 0);

protected:
	UPROPERTY(ReplicatedUsing=OnRep_CurrentRotationIndex, BlueprintReadOnly, Category="Rotation")
	int32 CurrentRotationIndex = 0;

	UPROPERTY(Replicated, BlueprintReadOnly, Category="Rotation")
	bool bIsRotating = false;

private:
    FRotator InitialRotation;
    FRotator TargetRotation;
    FTimerHandle RotationTimerHandle;
};