// AO_LavaMonsterCtrl.h

#pragma once

#include "CoreMinimal.h"
#include "AI/Controller/AO_AggressiveAICtrl.h"
#include "AO_LavaMonsterCtrl.generated.h"

class AAO_LavaMonster;

/**
 * 용암 몬스터 AI Controller
 * 
 * 역할:
 * - 용암 몬스터 전용 행동 제어
 * - 선공형 AI 로직 재사용 (추격/수색)
 * - Perception 처리 (시야/청각)
 */
UCLASS()
class AO_API AAO_LavaMonsterCtrl : public AAO_AggressiveAICtrl
{
	GENERATED_BODY()

public:
	AAO_LavaMonsterCtrl();

	// 소유한 LavaMonster 가져오기
	UFUNCTION(BlueprintCallable, Category = "AO|AI|LavaMonster")
	AAO_LavaMonster* GetLavaMonster() const;

	// 공격 가능 여부
	UFUNCTION(BlueprintCallable, Category = "AO|AI|LavaMonster")
	bool CanAttack() const;

protected:
	virtual void OnPossess(APawn* InPawn) override;
};

