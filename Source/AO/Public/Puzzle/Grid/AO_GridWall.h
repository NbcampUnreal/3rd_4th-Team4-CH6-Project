// HSJ : AO_GridWall.h
#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Puzzle/Grid/AO_GridManager.h"
#include "AO_GridWall.generated.h"

UENUM(BlueprintType)
enum class EWallOrientation : uint8
{
	Horizontal UMETA(DisplayName = "Horizontal"),
	Vertical   UMETA(DisplayName = "Vertical")
};

UCLASS()
class AO_API AAO_GridWall : public AActor
{
	GENERATED_BODY()

public:
	AAO_GridWall();

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Components")
	TObjectPtr<UStaticMeshComponent> WallMesh;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Grid")
	EWallOrientation WallOrientation = EWallOrientation::Horizontal;

	UPROPERTY(EditInstanceOnly, BlueprintReadWrite, Category="Grid")
	TObjectPtr<AAO_GridManager> GridManager;

	FIntPoint GetGridCell() const;
	EWallOrientation GetOrientation() const { return WallOrientation; }

protected:
	virtual void BeginPlay() override;

#if WITH_EDITOR
	virtual void PostEditMove(bool bFinished) override;
#endif

private:
	void UpdateWallTransform();
	bool bIsUpdatingTransform = false;
};