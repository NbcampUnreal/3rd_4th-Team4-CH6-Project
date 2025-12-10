// JSH: AO_StageInteractable.h

#pragma once

#include "CoreMinimal.h"
#include "Interaction/Base/AO_BaseInteractable.h"
#include "AO_StageInteractable.generated.h"

UENUM(BlueprintType)
enum class EAO_StageInteractType : uint8
{
	ExitToNextArea UMETA(DisplayName="ExitToNextArea"),
	ExitToLobby    UMETA(DisplayName="ExitToLobby"),
	// 추후: ActivateEvent, OpenDoor 등 추가 가능
};

UCLASS()
class AO_API AAO_StageInteractable : public AAO_BaseInteractable
{
	GENERATED_BODY()

public:
	AAO_StageInteractable(const FObjectInitializer& ObjectInitializer);

protected:
	virtual bool CanInteraction(const FAO_InteractionQuery& InteractionQuery) const override;
	virtual void OnInteractionSuccess_BP_Implementation(AActor* Interactor) override;

protected:
	UPROPERTY(EditAnywhere, Category="Stage")
	EAO_StageInteractType InteractType;

	UPROPERTY(EditAnywhere, Category="Stage")
	float RequiredFuel = 20.0f;
};
