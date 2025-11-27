// HSJ : AO_DecayHoldElement.h
#pragma once

#include "CoreMinimal.h"
#include "Puzzle/Element/AO_PuzzleElement.h"
#include "AO_DecayHoldElement.generated.h"

/**
 * 진행도 감소 방식 홀드 퍼즐 요소
 * 
 * - 홀드하면 진행도 증가, 놓으면 천천히 감소
 * - 목표 시간 도달 시 활성화
 */
UCLASS()
class AO_API AAO_DecayHoldElement : public AAO_PuzzleElement
{
	GENERATED_BODY()

public:
	AAO_DecayHoldElement(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());
	
	virtual bool CanInteraction(const FAO_InteractionQuery& InteractionQuery) const override;
	virtual void OnInteractActiveStarted(AActor* Interactor) override;
	virtual void OnInteractActiveEnded(AActor* Interactor) override;
	virtual void ResetToInitialState() override;

	UFUNCTION(BlueprintPure, Category="Puzzle|DecayHold")
	float GetHoldProgress() const { return CurrentHoldProgress; }

	UFUNCTION(BlueprintPure, Category="Puzzle|DecayHold")
	float GetHoldProgressPercent() const 
	{ 
		return PuzzleInteractionInfo.Duration > 0.f ? CurrentHoldProgress / PuzzleInteractionInfo.Duration : 0.f; 
	}

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="DecayHold|Reaction")
	TObjectPtr<class AAO_PuzzleReactionActor> LinkedReactionActor;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="DecayHold", meta=(ClampMin="0.1"))
	float DecayRate = 1.0f;

protected:
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	UPROPERTY(Replicated, BlueprintReadOnly, Category="DecayHold")
	float CurrentHoldProgress = 0.0f;

private:
	void UpdateProgress();
	void StartProgressTimer();
	void StopProgressTimer();
    
	bool IsAnyoneHolding() const;
	void HandleToggleCompletion();

	FTimerHandle ProgressTimerHandle;
    
	// HoldActive에서 Duration 도달 시 홀드 유지용
	UPROPERTY(Replicated)
	TArray<TWeakObjectPtr<AActor>> ManualHoldActors;
};