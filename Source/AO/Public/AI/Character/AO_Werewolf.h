// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AI/Base/AO_AggressiveAIBase.h"
#include "AO_Werewolf.generated.h"

class UAO_PackCoordComp;

/**
 * Werewolf (늑대인간)
 * - 빠른 이동 속도 (400 / 700)
 * - 무리 행동 (Pack Coordination)
 */
UCLASS()
class AO_API AAO_Werewolf : public AAO_AggressiveAIBase
{
	GENERATED_BODY()
	
public:
	AAO_Werewolf();

	virtual void PostInitializeComponents() override;

	// 무리 행동 컴포넌트 접근자
	UFUNCTION(BlueprintCallable, Category = "AO|AI|Werewolf")
	UAO_PackCoordComp* GetPackCoordComp() const { return PackCoordComp; }

protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AO|AI|Werewolf")
	TObjectPtr<UAO_PackCoordComp> PackCoordComp;
};
