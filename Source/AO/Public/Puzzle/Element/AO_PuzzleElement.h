// HSJ : AO_PuzzleElement.h
#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "Interaction/Actor/AO_WorldInteractable.h"
#include "AO_PuzzleElement.generated.h"

class AAO_PuzzleConditionChecker;
class UAO_InteractionDataAsset;

/**
 * - OneTime: 한 번만 활성화 (버튼 누르면 끝)
 * - Toggle: 토글 방식 (켜기/끄기 반복)
 * - HoldToggle: 홀딩 완료 시 토글
 * - HoldContinuous: 홀딩하는 동안만 활성화 (압력판)
 */
UENUM(BlueprintType)
enum class EPuzzleElementType : uint8
{
	OneTime         UMETA(DisplayName = "One Time"),
	Toggle          UMETA(DisplayName = "Toggle"),
	HoldToggle      UMETA(DisplayName = "Hold Toggle"),
	HoldContinuous  UMETA(DisplayName = "Hold Continuous")
};

/**
 * 퍼즐 요소 베이스 클래스
 * 
 * - 4가지 상호작용 타입 지원 (OneTime/Toggle/HoldToggle/HoldContinuous)
 * - InteractionDataAsset으로 상호작용 정보 프리셋 사용 가능
 * - 인스턴스별 메시/머티리얼 설정 가능 (한 클래스로 모든 퍼즐 요소 표현)
 * - PuzzleConditionChecker에 이벤트 태그 전송
 * 
 * 1. 플레이어 상호작용 → OnInteractionSuccess() 호출
 * 2. ElementType에 따라 bIsActivated 상태 변경
 * 3. BroadcastPuzzleEvent()로 ActivatedEventTag 전송
 * 4. LinkedChecker가 조건 검사
 * 
 */
UCLASS(Abstract)
class AO_API AAO_PuzzleElement : public AAO_WorldInteractable
{
	GENERATED_BODY()

public:
	AAO_PuzzleElement(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

	virtual FAO_InteractionInfo GetInteractionInfo(const FAO_InteractionQuery& InteractionQuery) const override;
	virtual bool CanInteraction(const FAO_InteractionQuery& InteractionQuery) const override;
	virtual void OnInteractionSuccess(AActor* Interactor) override;
	virtual void OnInteractActiveStarted(AActor* Interactor) override;
	virtual void OnInteractActiveEnded(AActor* Interactor) override;
	virtual void GetMeshComponents(TArray<UMeshComponent*>& OutMeshComponents) const override;

	UFUNCTION(BlueprintCallable, Category="Puzzle")
	void BroadcastPuzzleEvent(bool bActivate);

	// 외부에서 상태 강제 변경
	UFUNCTION(BlueprintCallable, Category="Puzzle")
	void SetActivationState(bool bNewState);

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Components")
	TObjectPtr<UStaticMeshComponent> MeshComponent;

protected:
	virtual void BeginPlay() override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

#if WITH_EDITOR
	virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;
#endif

	UFUNCTION()
	virtual void OnRep_IsActivated();

	UFUNCTION(BlueprintImplementableEvent, Category="Puzzle")
	void OnElementStateChanged(bool bIsActive);

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Puzzle")
	EPuzzleElementType ElementType = EPuzzleElementType::OneTime;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Puzzle")
	FGameplayTag ActivatedEventTag;		// 활성화 시 전송할 태그

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Puzzle", 
		meta=(EditCondition="ElementType == EPuzzleElementType::Toggle || ElementType == EPuzzleElementType::HoldToggle || ElementType == EPuzzleElementType::HoldContinuous", 
		EditConditionHides))
	FGameplayTag DeactivatedEventTag;	// 비활성화 시 전송할 태그

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Puzzle")
	TObjectPtr<AAO_PuzzleConditionChecker> LinkedChecker;

	UPROPERTY(ReplicatedUsing=OnRep_IsActivated, BlueprintReadOnly, Category="Puzzle")
	bool bIsActivated = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Puzzle|Interaction")
	bool bUseInteractionDataAsset = false;

	// DataAsset에 있는 정보 사용 여부
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Puzzle|Interaction",
		meta=(EditCondition="bUseInteractionDataAsset", EditConditionHides))
	TObjectPtr<UAO_InteractionDataAsset> InteractionDataAsset;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Puzzle|Interaction", 
		meta=(EditCondition="!bUseInteractionDataAsset", EditConditionHides))
	FAO_InteractionInfo PuzzleInteractionInfo;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Puzzle|Visual")
	TObjectPtr<UStaticMesh> PuzzleMesh;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Puzzle|Visual")
	TArray<TObjectPtr<UMaterialInterface>> PuzzleMaterials;

private:
	UPROPERTY()
	TObjectPtr<AActor> CurrentHolder;	// 현재 홀딩 중인 액터
};