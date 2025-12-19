// HSJ : AO_TorchPuzzleElement.cpp
#include "Puzzle/Actor/Torch/AO_TorchPuzzleElement.h"

#include "AO_Log.h"
#include "Net/UnrealNetwork.h"
#include "Puzzle/Checker/AO_PuzzleConditionChecker.h"

AAO_TorchPuzzleElement::AAO_TorchPuzzleElement(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	ElementType = EPuzzleElementType::Toggle;
	bHandleToggleInOnInteractionSuccess = true;
}

void AAO_TorchPuzzleElement::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(AAO_TorchPuzzleElement, bIsTorchLit);
}

void AAO_TorchPuzzleElement::BeginPlay()
{
	Super::BeginPlay();
    
	// 초기 상태가 정답인지 체크
	if (HasAuthority())
	{
		CheckAndBroadcastCorrectState();
	}
}

void AAO_TorchPuzzleElement::OnInteractionSuccess(AActor* Interactor)
{
	Super::OnInteractionSuccess(Interactor);
    
	if (!HasAuthority())
	{
		return;
	}
    
	// 횃불 상태 토글
	bIsTorchLit = !bIsTorchLit;
    
	CheckAndBroadcastCorrectState();
}

void AAO_TorchPuzzleElement::CheckAndBroadcastCorrectState()
{
	if (!LinkedChecker)
	{
		return;
	}
    
	if (!TorchCorrectTag.IsValid())
	{
		return;
	}
    
	// bIsTorchLit 기준으로 체크
	bool bIsCorrect = (bIsTorchLit == bCorrectStateIsLit);
    
	if (bIsCorrect)
	{
		LinkedChecker->OnPuzzleEvent(TorchCorrectTag, GetInstigator());
	}
	else
	{
		LinkedChecker->RemovePuzzleEvent(TorchCorrectTag);
	}
}