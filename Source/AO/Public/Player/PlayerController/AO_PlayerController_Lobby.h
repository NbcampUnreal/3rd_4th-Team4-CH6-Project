// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AO_PlayerController_InGameBase.h"
#include "AO_PlayerController_Lobby.generated.h"

/**
 * 
 */
UCLASS()
class AO_API AAO_PlayerController_Lobby : public AAO_PlayerController_InGameBase
{
	GENERATED_BODY()

public:
	AAO_PlayerController_Lobby();

protected:
	virtual void BeginPlay() override;
};
