// JSH: AO_GameInstance.h

#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstance.h"
#include "AO_GameInstance.generated.h"

/**
 * 
 */
UCLASS()
class AO_API UAO_GameInstance : public UGameInstance
{
	GENERATED_BODY()

public:
	// 스테이지/휴게공간 사이를 오갈 때 유지할 연료 값 (서버 기준)
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Train")
	float SharedTrainFuel = 0.0f;
};
