// JSH : AO_RestInteractable.h

#pragma once

#include "CoreMinimal.h"
#include "Interaction/Base/AO_BaseInteractable.h"
#include "AO_RestInteractable.generated.h"

UENUM(BlueprintType)
enum class EAO_RestInteractType : uint8
{
	ProceedToNextStage	UMETA(DisplayName="ProceedToNextStage"),
	// 추후: ReturnToLobby 등 필요 시 추가
};

UCLASS()
class AO_API AAO_RestInteractable : public AAO_BaseInteractable
{
	GENERATED_BODY()

public:
	AAO_RestInteractable(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

protected:
	virtual bool CanInteraction(const FAO_InteractionQuery& InteractionQuery) const override;
	virtual void OnInteractionSuccess_BP_Implementation(AActor* Interactor) override;

protected:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="AO|Rest")
	EAO_RestInteractType InteractType;
};
