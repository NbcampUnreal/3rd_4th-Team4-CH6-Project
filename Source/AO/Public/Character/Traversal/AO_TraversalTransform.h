// AO_TraversalTransform.h - KH

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "AO_TraversalTransform.generated.h"

UINTERFACE(MinimalAPI, BlueprintType)
class UAO_TraversalTransform : public UInterface
{
	GENERATED_BODY()
};

class AO_API IAO_TraversalTransform
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Traversal")
	void SetTraversalTransform(const FTransform& TraversalTransform);

	virtual void SetTraversalTransform_Implementation(const FTransform& TraversalTransform) = 0;

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Traversal", meta = (BlueprintThreadSafe))
	FTransform GetTraversalTransform();

	virtual FTransform GetTraversalTransform_Implementation() = 0;
};
