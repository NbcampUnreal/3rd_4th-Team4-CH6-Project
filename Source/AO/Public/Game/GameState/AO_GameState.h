// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameState.h"
#include "AO_GameState.generated.h"

/**
 * 
 */
UCLASS()
class AO_API AAO_GameState : public AGameState
{
	GENERATED_BODY()
	
public:
	AAO_GameState();

public:
	UPROPERTY(ReplicatedUsing = OnRep_SharedReviveCount, VisibleAnywhere, BlueprintReadOnly, Category = "AO|Revive")
	int32 SharedReviveCount;

protected:
	UFUNCTION()
	void OnRep_SharedReviveCount();

public:
	void SetSharedReviveCount(int32 InValue);

	UFUNCTION(BlueprintCallable, Category = "AO|Revive")
	int32 GetSharedReviveCount() const;

public:
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
};
