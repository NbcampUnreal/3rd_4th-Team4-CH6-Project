// HSJ : AO_Interface_PuzzleElement.h
#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "AO_Interface_PuzzleElement.generated.h"

UINTERFACE(MinimalAPI, BlueprintType)
class UAO_Interface_PuzzleElement : public UInterface
{
	GENERATED_BODY()
};

/**
 * 퍼즐 요소가 구현해야 하는 공통 인터페이스
 * - Checker에서 리셋/비활성화 제어용
 */
class AO_API IAO_Interface_PuzzleElement
{
	GENERATED_BODY()

public:
	// 초기 상태로 복구
	virtual void ResetToInitialState() = 0;

	// 상호작용 활성화/비활성화
	virtual void SetInteractionEnabled(bool bEnabled) = 0;

	// 현재 상호작용 가능 여부
	virtual bool IsInteractionEnabled() const = 0;
};