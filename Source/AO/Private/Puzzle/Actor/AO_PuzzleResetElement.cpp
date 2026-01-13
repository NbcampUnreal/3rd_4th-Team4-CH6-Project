// HSJ : AO_PuzzleResetElement.cpp
#include "Puzzle/Actor/AO_PuzzleResetElement.h"
#include "Puzzle/Checker/AO_PuzzleConditionChecker.h"
#include "AO_Log.h"

AAO_PuzzleResetElement::AAO_PuzzleResetElement(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	ElementType = EPuzzleElementType::OneTime;
	bHandleToggleInOnInteractionSuccess = false;
}

void AAO_PuzzleResetElement::OnInteractionSuccess(AActor* Interactor)
{
	Super::OnInteractionSuccess(Interactor);

	if (!HasAuthority())
	{
		return;
	}

	if (!LinkedChecker)
	{
		return;
	}

	LinkedChecker->ResetPuzzle();
}