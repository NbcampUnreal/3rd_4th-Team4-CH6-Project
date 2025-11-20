// HSJ : AO_Interface_Interactable.h
#pragma once

#include "CoreMinimal.h"
#include "AO_InteractionInfo.h"
#include "AO_InteractionQuery.h"
#include "UObject/Interface.h"
#include "AO_Interface_Interactable.generated.h"

UINTERFACE(MinimalAPI, BlueprintType, meta=(CannotImplementInterfaceInBlueprint))
class UAO_Interface_Interactable : public UInterface
{
	GENERATED_BODY()
};

/**
 * 상호작용 가능한 오브젝트가 구현해야 하는 인터페이스
 * 
 * - GetInteractionInfo: 상호작용 정보 제공 (UI 표시용)
 * - CanInteraction: 현재 상호작용 가능 여부 체크
 * - GetMeshComponents: 하이라이트할 메시 컴포넌트 제공
 * 
 * 구현 예:
 * - AAO_WorldInteractable: 월드에 배치된 상호작용 오브젝트
 * - AAO_PuzzleElement: 퍼즐 요소
 */
class IAO_Interface_Interactable
{
	GENERATED_BODY()

public:
	// 🆕 상호작용 정보 반환 (GetPreInteractionInfo + GatherPostInteractionInfos 통합)
	virtual FAO_InteractionInfo GetInteractionInfo(const FAO_InteractionQuery& InteractionQuery) const 
	{ 
		return FAO_InteractionInfo(); 
	}

	// 하이라이트할 메시 컴포넌트 반환, CustomDepth 렌더링에 사용 (외곽선 표시)
	UFUNCTION(BlueprintCallable)
	virtual void GetMeshComponents(TArray<UMeshComponent*>& OutMeshComponents) const 
	{ }
	
	// 현재 상호작용 가능 여부 체크
	UFUNCTION(BlueprintCallable)
	virtual bool CanInteraction(const FAO_InteractionQuery& InteractionQuery) const 
	{ 
		return true; 
	}
};