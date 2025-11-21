// AO_TraversableComponent.h - KH

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Character/Traversal/AO_TraversalTypes.h"
#include "AO_TraversableComponent.generated.h"

class USplineComponent;

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class AO_API UAO_TraversableComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UAO_TraversableComponent();
	
protected:
	virtual void OnRegister() override;

public:
	void GetLedgeTransforms(FVector& HitLocation, FVector& ActorLocation, FTraversalCheckResult& TraversalResult);
	
protected:
	UPROPERTY(BlueprintReadOnly, Category = "Traversal")
	TArray<USplineComponent*> Ledges;
	UPROPERTY(BlueprintReadOnly, Category = "Traversal")
	TMap<USplineComponent*, USplineComponent*> OppositeLedges;
	UPROPERTY(BlueprintReadOnly, Category = "Traversal")
	float MinLedgeWidth = 60.f;

private:
	void ScanSplines();
	USplineComponent* FindLedgeClosestToActor(FVector& ActorLocation);
};
