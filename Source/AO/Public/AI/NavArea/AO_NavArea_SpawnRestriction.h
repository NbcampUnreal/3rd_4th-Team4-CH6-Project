// AO_NavArea_SpawnRestriction.h

#pragma once

#include "CoreMinimal.h"
#include "NavAreas/NavArea.h"
#include "AO_NavArea_SpawnRestriction.generated.h"

/**
 * 스폰 금지 NavArea
 * NavModifierVolume에서 이 AreaClass를 지정하면 해당 영역에는 AI가 스폰되지 않습니다.
 */
UCLASS()
class AO_API UAO_NavArea_SpawnRestriction : public UNavArea
{
	GENERATED_BODY()

public:
	UAO_NavArea_SpawnRestriction(const FObjectInitializer& ObjectInitializer);
};

