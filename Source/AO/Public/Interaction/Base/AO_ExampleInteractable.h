// HSJ : AO_ExampleInteractable.h
#pragma once

#include "CoreMinimal.h"
#include "Interaction/Base/AO_BaseInteractable.h"
#include "AO_ExampleInteractable.generated.h"

UCLASS()
class AO_API AAO_ExampleInteractable : public AAO_BaseInteractable
{
	GENERATED_BODY()

public:
	AAO_ExampleInteractable(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

	virtual bool CanInteraction(const FAO_InteractionQuery& InteractionQuery) const override;

protected:
	// 상호작용 성공 시 호출 (Interactor: 상호작용한 액터)
	virtual void OnInteractionSuccess_BP_Implementation(AActor* Interactor) override;

private:
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
};