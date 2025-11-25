// HSJ : AO_OverwatchInspectionPuzzle.h
#pragma once

#include "CoreMinimal.h"
#include "Interaction/Interface/AO_Interface_InspectionCameraTypes.h"
#include "Puzzle/Element/AO_InspectionPuzzle.h"
#include "AO_OverwatchInspectionPuzzle.generated.h"

/**
 * 외부 액터 메시 매핑 구조체
 * - 레벨에 배치된 다른 액터의 메시도 클릭 가능하게 연결
 */
USTRUCT(BlueprintType)
struct FAO_ExternalMeshMapping
{
    GENERATED_BODY()

    // 연결할 외부 액터
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mapping")
    TObjectPtr<AActor> TargetActor;

    // 해당 액터 내 클릭 가능한 컴포넌트 이름
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mapping")
    FName ComponentName;

    // 클릭 시 발생할 태그
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mapping")
    FGameplayTag ActivatedTag;
};

/**
 * 오버워치 Inspection 퍼즐
 * 
 * - 하늘에서 내려다보는 카메라 뷰
 * - WASD로 카메라 이동 (Z축과 회전 고정)
 * - 카메라 이동 범위 제한
 * - 외부 액터 메시 연결 가능
 * 
 */
UCLASS()
class AO_API AAO_OverwatchInspectionPuzzle : public AAO_InspectionPuzzle,
    public IAO_Interface_InspectionCameraProvider
{
    GENERATED_BODY()

public:
    AAO_OverwatchInspectionPuzzle(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

    // IAO_Interface_Inspectable
    virtual void OnInspectionMeshClicked(UPrimitiveComponent* ClickedComponent) override;

    // IAO_Interface_InspectionCameraProvider
    virtual FAO_InspectionCameraSettings GetInspectionCameraSettings() const override;
    virtual bool IsValidClickTarget(AActor* HitActor, UPrimitiveComponent* Component) const override;

protected:
    virtual void BeginPlay() override;
    virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

private:
    FGameplayTag FindTagForExternalComponent(AActor* HitActor, UPrimitiveComponent* Component) const;

public:
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Overwatch|Camera")
    EInspectionCameraMode CameraMode = EInspectionCameraMode::WorldAbsolute;

    // 월드 절대좌표 카메라 위치 (CameraMode가 WorldAbsolute일 때 사용)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Overwatch|Camera",
        meta = (EditCondition = "CameraMode == EInspectionCameraMode::WorldAbsolute", EditConditionHides))
    FVector AbsoluteCameraLocation = FVector(0.f, 0.f, 1000.f);

    // 월드 절대좌표 카메라 회전 (CameraMode가 WorldAbsolute일 때 사용)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Overwatch|Camera",
        meta = (EditCondition = "CameraMode == EInspectionCameraMode::WorldAbsolute", EditConditionHides))
    FRotator AbsoluteCameraRotation = FRotator(-90.f, 0.f, 0.f);

    // WASD 카메라 이동 활성화
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Overwatch|Movement")
    bool bEnableCameraMovement = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Overwatch|Movement",
        meta = (EditCondition = "bEnableCameraMovement", ClampMin = "10.0", UIMin = "10.0"))
    float CameraMovementSpeed = 500.f;

    // 초기 카메라 위치 기준 이동 가능한 범위
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Overwatch|Movement",
        meta = (EditCondition = "bEnableCameraMovement"))
    FVector MovementBoundsExtent = FVector(500.f, 500.f, 0.f);

    // 외부 액터 메시 매핑 (레벨에 배치된 다른 액터의 메시와 연결)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Overwatch|ExternalMesh")
    TArray<FAO_ExternalMeshMapping> ExternalMeshMappings;

protected:
    // 복제용 외부 매핑
    UPROPERTY(Replicated)
    TArray<FAO_ExternalMeshMapping> ReplicatedExternalMappings;
};