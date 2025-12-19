// HSJ : AO_GridManager.h
#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "GameFramework/Actor.h"
#include "AO_GridManager.generated.h"

class AAO_PushableRockElement;
class AAO_GridWall;
enum class EWallOrientation : uint8;

// 격자 방향
UENUM(BlueprintType)
enum class EGridDirection : uint8
{
    North,
    East,
    South,
    West
};

UCLASS()
class AO_API AAO_GridManager : public AActor
{
    GENERATED_BODY()

public:
    AAO_GridManager();

    UFUNCTION(BlueprintPure, Category="Grid")
    FIntPoint WorldToGrid(const FVector& WorldLocation) const;

    UFUNCTION(BlueprintPure, Category="Grid")
    FVector GridToWorld(const FIntPoint& GridCoord) const;

    UFUNCTION(BlueprintPure, Category="Grid")
    bool IsRockAt(const FIntPoint& GridCoord) const;

    UFUNCTION(BlueprintPure, Category="Grid")
    bool IsValidGridCoord(const FIntPoint& GridCoord) const;

    UFUNCTION(BlueprintCallable, Category="Grid")
    bool CanRockMove(const FIntPoint& RockCoord, EGridDirection Direction) const;

    void RegisterRock(AAO_PushableRockElement* Rock, const FIntPoint& GridCoord);
    void UnregisterRock(AAO_PushableRockElement* Rock);
    void UpdateRockPosition(AAO_PushableRockElement* Rock, const FIntPoint& OldCoord, const FIntPoint& NewCoord);

    void RegisterWall(AAO_GridWall* Wall);
    void UnregisterWall(AAO_GridWall* Wall);

    UFUNCTION(BlueprintPure, Category="Grid")
    static FIntPoint GetDirectionVector(EGridDirection Direction);

    UFUNCTION(BlueprintPure, Category="Grid")
    static EGridDirection GetDirectionFromVector(const FVector& WorldDirection);

	bool HasWallBetween(const FIntPoint& From, const FIntPoint& To) const;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Grid", meta=(ClampMin="10.0"))
	float CellSize = 200.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Grid", meta=(ClampMin="1"))
	FIntPoint GridSize = FIntPoint(10, 10);

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Grid")
	FVector GridOrigin = FVector::ZeroVector;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Grid|Debug")
	bool bShowDebugGrid = true;

protected:
    virtual void BeginPlay() override;

#if WITH_EDITOR
    virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;
#endif

private:
	void DrawDebugGrid();
	
    UPROPERTY()
    TMap<FIntPoint, TObjectPtr<AAO_PushableRockElement>> RockMap;

    UPROPERTY()
    TArray<TObjectPtr<AAO_GridWall>> Walls;
};