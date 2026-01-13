// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AO_ItemDataAsset.h"
#include "AO_struct_FItemBase.h"
#include "UObject/Object.h"
#include "AO_ItemManager.generated.h"

/**
 * 
 */
UCLASS()
class AO_API UAO_ItemManager : public UObject
{
	GENERATED_BODY()

public:
	static UAO_ItemManager* Get();

	void Initialize(UAO_ItemDataAsset* InDatabase);

	bool GetItemData(FName ItemID, FAO_struct_FItemBase& OutData);

private:
	UAO_ItemDataAsset* Database;
	UDataTable* LoadedTable;
};
