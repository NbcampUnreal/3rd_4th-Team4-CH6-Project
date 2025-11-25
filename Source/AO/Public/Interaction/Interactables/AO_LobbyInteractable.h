#pragma once

#include "CoreMinimal.h"
#include "Interaction/Base/AO_BaseInteractable.h"
#include "AO_LobbyInteractable.generated.h"

UENUM(BlueprintType)
enum class EAO_LobbyInteractType : uint8
{
	ReadyToggle,
	StartGame,
	InviteFriends,
	Wardrobe
};

UCLASS()
class AO_API AAO_LobbyInteractable : public AAO_BaseInteractable
{
	GENERATED_BODY()

public:
	AAO_LobbyInteractable(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

protected:
	virtual void OnInteractionSuccess_BP_Implementation(AActor* Interactor) override;
	virtual bool CanInteraction(const FAO_InteractionQuery& InteractionQuery) const override;

public:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="AO|Lobby")
	EAO_LobbyInteractType InteractType;

	UPROPERTY(EditInstanceOnly, Category="AO|Lobby")
	AActor* PreviewSpawnPoint;
};
