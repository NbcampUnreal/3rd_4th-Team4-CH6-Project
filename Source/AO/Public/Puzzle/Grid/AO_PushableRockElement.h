// HSJ : AO_PushableRockElement.h
#pragma once

#include "CoreMinimal.h"
#include "Puzzle/Element/AO_PuzzleElement.h"
#include "Puzzle/Grid/AO_GridManager.h"
#include "AO_PushableRockElement.generated.h"

UCLASS()
class AO_API AAO_PushableRockElement : public AAO_PuzzleElement
{
    GENERATED_BODY()

public:
    AAO_PushableRockElement(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

	virtual void OnInteractionSuccess(AActor* Interactor) override;
	virtual void ResetToInitialState() override;

    UFUNCTION(BlueprintCallable, Category="Rock")
    bool TryPush(EGridDirection Direction);

    UFUNCTION(BlueprintPure, Category="Rock")
    FIntPoint GetGridCoord() const { return CurrentGridCoord; }

    UFUNCTION(BlueprintPure, Category="Rock")
    bool IsAtGoalCell() const;

protected:
    virtual void BeginPlay() override;
    virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
    virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

    UFUNCTION()
    void OnRep_CurrentGridCoord();

private:
    void StartMoveAnimation(const FVector& InTargetLocation);
    void UpdateMoveAnimation();
    EGridDirection GetPushDirectionFromInteractor(AActor* Interactor) const;

	bool CheckWallsAtTarget(const FVector& TargetLocation);

public:
	UPROPERTY(EditInstanceOnly, BlueprintReadWrite, Category="Grid")
	TObjectPtr<AAO_GridManager> GridManager;
	
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Grid|Animation", meta=(ClampMin="0.1"))
    float MoveSpeed = 5.0f;

    // 정답 목표 셀
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Grid|Goal")
    TArray<FIntPoint> GoalCells;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Grid|Reset")
	FVector ResetPushOffset = FVector(200.0f, 0.0f, 0.0f);

protected:
    UPROPERTY(ReplicatedUsing=OnRep_CurrentGridCoord, BlueprintReadOnly, Category="Grid")
    FIntPoint CurrentGridCoord;

    UPROPERTY(BlueprintReadOnly, Category="Grid")
    FIntPoint InitialGridCoord;

private:
    FTimerHandle MoveAnimationTimer;
    FVector TargetWorldLocation;
    bool bIsMoving = false;

	// 바위 이동 방향
	FVector MoveDirection;

	bool bWallForward = false;
	bool bWallLeft = false;
	bool bWallRight = false;
};