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
	Tutorial       UMETA(DisplayName="Tutorial"),
	// 추후: ActivateEvent, OpenDoor 등 추가 가능
};

UCLASS()
class AO_API AAO_StageInteractable : public AAO_BaseInteractable
{
	GENERATED_BODY()

public:
	AAO_StageInteractable(const FObjectInitializer& ObjectInitializer);

protected:
	virtual FAO_InteractionInfo GetInteractionInfo(const FAO_InteractionQuery& InteractionQuery) const override;
	virtual void OnInteractionSuccess_BP_Implementation(AActor* Interactor) override;
	
	void GetCurrentConditions(bool& bOutHasFuel, bool& bOutHasClues) const;

protected:
	UPROPERTY(EditAnywhere, Category="Stage")
	EAO_StageInteractType InteractType;

	UPROPERTY(EditAnywhere, Category="Stage")
	float RequiredFuel = 20.0f;

	UPROPERTY(EditAnywhere, Category="Stage", meta=(EditCondition="InteractType==EAO_StageInteractType::Tutorial", EditConditionHides))
	FName TutorialExitLevelName = TEXT("LV_MainMenu");
};
