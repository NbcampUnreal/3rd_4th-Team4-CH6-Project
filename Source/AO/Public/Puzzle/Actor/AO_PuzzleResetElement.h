// HSJ : AO_PuzzleResetElement.h
#pragma once

#include "CoreMinimal.h"
#include "Puzzle/Element/AO_PuzzleElement.h"
#include "AO_PuzzleResetElement.generated.h"

/**
 * 퍼즐 리셋 버튼
 * - 상호작용 시 연결된 Checker의 모든 Element를 초기 상태로 리셋
 */
UCLASS()
class AO_API AAO_PuzzleResetElement : public AAO_PuzzleElement
{
	GENERATED_BODY()

public:
	AAO_PuzzleResetElement(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

	virtual void OnInteractionSuccess(AActor* Interactor) override;
};