// AO_Area_SpawnRestriction.h

#pragma once

#include "CoreMinimal.h"
#include "NavModifierVolume.h"
#include "AO_Area_SpawnRestriction.generated.h"

/**
 * 스폰 금지 영역
 * 이 볼륨 내부에는 AI가 스폰되지 않습니다.
 */
UCLASS()
class AO_API AAO_Area_SpawnRestriction : public ANavModifierVolume
{
	GENERATED_BODY()

public:
	AAO_Area_SpawnRestriction(const FObjectInitializer& ObjectInitializer);
};

