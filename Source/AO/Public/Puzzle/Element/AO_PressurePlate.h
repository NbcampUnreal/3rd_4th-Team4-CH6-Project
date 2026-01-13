// HSJ : AO_PressurePlate.h
#pragma once

#include "CoreMinimal.h"
#include "Puzzle/Element/AO_PuzzleElement.h"
#include "AO_PressurePlate.generated.h"

class UBoxComponent;
class AAO_PuzzleReactionActor;
class AAO_MasterItem;

/**
 * Overlap 기반 압력판
 * 
 * - 플레이어가 올라가면 활성화, 내려가면 비활성화
 * - 여러 명이 올라가 있을 때 모두 내려가야 비활성화
 */
UCLASS()
class AO_API AAO_PressurePlate : public AAO_PuzzleElement
{
	GENERATED_BODY()

public:
	AAO_PressurePlate(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

	virtual void GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const override;

	virtual bool CanInteraction(const FAO_InteractionQuery& InteractionQuery) const override;
	virtual void ResetToInitialState() override;
	virtual void OnRep_IsActivated() override;

protected:
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

	UFUNCTION()
	void OnOverlapBegin(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, 
		UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

	UFUNCTION()
	void OnOverlapEnd(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, 
		UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);

	void StartPlateAnimation();
	void UpdatePlateAnimation();

	void StartProgressTimer();
	void StopProgressTimer();
	void UpdateProgress();

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Components")
	TObjectPtr<UBoxComponent> OverlapTrigger;

	// Overlap 중인 액터들 추적
	UPROPERTY()
	TArray<TObjectPtr<AActor>> OverlappingActors;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="PressurePlate|Animation", meta=(ClampMin="1.0"))
	float PressDepth = 20.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="PressurePlate|Reaction")
	TArray<TObjectPtr<AAO_PuzzleReactionActor>> LinkedReactionActors;

	// 진행도 변화 속도
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="PressurePlate|Reaction", meta=(ClampMin="0.1"))
	float ProgressSpeed = 2.0f;

	// 현재 진행도 (0.0 ~ 1.0)
	UPROPERTY(Replicated, BlueprintReadOnly, Category="PressurePlate|Reaction")
	float CurrentProgress = 0.0f;

private:
	FVector InitialMeshLocation;
	FVector TargetMeshLocation;
	FTimerHandle PlateAnimationTimerHandle;
	FTimerHandle ProgressTimerHandle;
};