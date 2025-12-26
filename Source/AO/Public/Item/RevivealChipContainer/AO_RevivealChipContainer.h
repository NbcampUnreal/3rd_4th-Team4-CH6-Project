#pragma once

#include "CoreMinimal.h"
#include "Interaction/Base/AO_BaseInteractable.h"
#include "AO_RevivealChipContainer.generated.h"

UCLASS()
class AO_API AAO_RevivealChipContainer : public AAO_BaseInteractable
{
	GENERATED_BODY()

public:
	AAO_RevivealChipContainer();

protected:
	virtual void BeginPlay() override;
	virtual void OnInteractionSuccess(AActor* Interactor) override;
};
