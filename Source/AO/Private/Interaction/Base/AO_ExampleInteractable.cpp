// HSJ : AO_ExampleInteractable.cpp
#include "Interaction/Base/AO_ExampleInteractable.h"
#include "AO_Log.h"
#include "Net/UnrealNetwork.h"

AAO_ExampleInteractable::AAO_ExampleInteractable(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	InteractionTitle = FText::FromString("Sample");
	InteractionContent = FText::FromString("Press F Button");
	InteractionDuration = 0.0f;
}

void AAO_ExampleInteractable::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
}

bool AAO_ExampleInteractable::CanInteraction(const FAO_InteractionQuery& InteractionQuery) const
{
	if (!Super::CanInteraction(InteractionQuery))
		return false;

	// 특정 조건(bool변수 등)을 걸어서 return false를 반환하면 상호작용을 막을 수 있음

	return true;
}

// 상호작용 성공 시 호출 (Interactor: 상호작용한 액터)
void AAO_ExampleInteractable::OnInteractionSuccess_BP_Implementation(AActor* Interactor)
{
	Super::OnInteractionSuccess_BP_Implementation(Interactor);

	if (!HasAuthority()) return;

	// 로직 작성
}