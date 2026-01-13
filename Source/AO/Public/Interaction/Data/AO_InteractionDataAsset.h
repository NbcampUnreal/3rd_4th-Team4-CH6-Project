// HSJ : AO_InteractionDataAsset.h
#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "Interaction/Interface/AO_InteractionInfo.h"
#include "AO_InteractionDataAsset.generated.h"

/**
 * 재사용 가능한 상호작용 프리셋
 * 예: 버튼 누르기, 레버 당기기, 문 열기 등
 */

UCLASS(BlueprintType)
class AO_API UAO_InteractionDataAsset : public UPrimaryDataAsset
{
	GENERATED_BODY()

public:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Interaction")
	FAO_InteractionInfo InteractionInfo;
	
	// 간단한 설명 (에디터에서 구분용)
	UPROPERTY(EditDefaultsOnly, Category="Interaction")
	FText Description;
};
