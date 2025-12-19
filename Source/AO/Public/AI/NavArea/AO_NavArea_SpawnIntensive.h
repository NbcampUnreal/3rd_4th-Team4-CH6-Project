// AO_NavArea_SpawnIntensive.h

#pragma once

#include "CoreMinimal.h"
#include "NavAreas/NavArea.h"
#include "AO_NavArea_SpawnIntensive.generated.h"

/**
 * 집중 스폰 NavArea
 * NavModifierVolume에서 이 AreaClass를 지정하면 해당 영역에서는 더 자주 스폰됩니다.
 */
UCLASS()
class AO_API UAO_NavArea_SpawnIntensive : public UNavArea
{
	GENERATED_BODY()

public:
	UAO_NavArea_SpawnIntensive(const FObjectInitializer& ObjectInitializer);
};

