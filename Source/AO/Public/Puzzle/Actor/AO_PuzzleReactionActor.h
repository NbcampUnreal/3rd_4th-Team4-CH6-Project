// HSJ : AO_PuzzleReactionActor.h
#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "AbilitySystemInterface.h"
#include "GameplayTagContainer.h"
#include "AO_PuzzleReactionActor.generated.h"

class UAbilitySystemComponent;
class UStaticMeshComponent;

/**
 * 퍼즐 반응 모드
 * - OneTime: 한 번만 변화 (퍼즐 성공 시 문 열림)
 * - Toggle: Tag 추가 시 열림, 제거 시 닫힘 (퍼즐 토글에 반응)
 * - HoldActive: DecayHoldElement의 진행도에 따라 실시간 변화
 */
UENUM(BlueprintType)
enum class EPuzzleReactionMode : uint8
{
    OneTime      UMETA(DisplayName = "One Time"),
    Toggle       UMETA(DisplayName = "Toggle"),
    HoldActive   UMETA(DisplayName = "Hold Active")
};

UCLASS()
class AO_API AAO_PuzzleReactionActor : public AActor, public IAbilitySystemInterface
{
    GENERATED_BODY()

public:
    AAO_PuzzleReactionActor();

    virtual UAbilitySystemComponent* GetAbilitySystemComponent() const override;

    // HoldActive 모드, 외부에서 진행도 설정 (0.0 ~ 1.0)
    UFUNCTION(BlueprintCallable, Category="Reaction")
    void SetProgress(float Progress);

    virtual void ActivateReaction();
	virtual void DeactivateReaction();

protected:
    virtual void BeginPlay() override;
    virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
    virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

    UFUNCTION()
    virtual void OnRep_IsActivated();

    UFUNCTION()
    virtual void OnRep_TargetProgress();

private:
    void OnTriggerTagChanged(const FGameplayTag Tag, int32 NewCount);
    
    // Transform 애니메이션 업데이트 (타이머 콜백)
    void UpdateTransform();
    bool IsRotatorNearlyEqual(const FRotator& A, const FRotator& B, float Tolerance = 0.5f) const;

public:
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Components")
    TObjectPtr<USceneComponent> RootComp;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Components")
    TObjectPtr<UStaticMeshComponent> MeshComponent;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Abilities")
    TObjectPtr<UAbilitySystemComponent> AbilitySystemComponent;

	UPROPERTY(EditInstanceOnly, BlueprintReadWrite, Category="Reaction|Attached Actors")
	TArray<TObjectPtr<AActor>> AttachedActors;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Reaction")
    EPuzzleReactionMode ReactionMode = EPuzzleReactionMode::OneTime;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Reaction")
    FGameplayTag TriggerTag;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Reaction|Transform")
    bool bUseLocation = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Reaction|Transform", 
        meta=(EditCondition="bUseLocation", EditConditionHides))
    FVector TargetRelativeLocation = FVector(0, 0, 200);

    // 회전 변화 사용 여부
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Reaction|Transform")
    bool bUseRotation = false;

    // 목표 상대 회전
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Reaction|Transform",
        meta=(EditCondition="bUseRotation", EditConditionHides))
    FRotator TargetRelativeRotation = FRotator::ZeroRotator;

    // Transform 애니메이션 속도
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Reaction|Animation", meta=(ClampMin="0.1"))
    float TransformSpeed = 2.0f;

protected:
    UPROPERTY(ReplicatedUsing=OnRep_IsActivated, BlueprintReadOnly, Category="Reaction")
    bool bIsActivated = false;

    // 목표 진행도 (HoldActive용, 0.0 ~ 1.0)
    UPROPERTY(ReplicatedUsing=OnRep_TargetProgress, BlueprintReadOnly, Category="Reaction")
    float TargetProgress = 0.0f;

	FTimerHandle TransformTimerHandle;

private:
    FVector InitialLocation;
    FRotator InitialRotation;

	TMap<TObjectPtr<AActor>, FTransform> AttachedActorOffsets;

	void UpdateAttachedActors();
};