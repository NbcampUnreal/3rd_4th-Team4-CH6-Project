// Fill out your copyright notice in the Description page of Project Settings.


#include "Item/AO_ItemManager.h"

UAO_ItemManager* Singleton = nullptr;

UAO_ItemManager* UAO_ItemManager::Get()
{
	if (!Singleton)
		Singleton = NewObject<UAO_ItemManager>();

	return Singleton;
}

void UAO_ItemManager::Initialize(UAO_ItemDataAsset* InDatabase)
{
	Database = InDatabase;

	if (Database && !Database->ItemDataTable.IsNull())
		LoadedTable = Database->ItemDataTable.LoadSynchronous();
}

bool UAO_ItemManager::GetItemData(FName ItemID, FAO_struct_FItemBase& OutData)
{
	if (!LoadedTable)
		return false;

	FAO_struct_FItemBase* Row = LoadedTable->FindRow<FAO_struct_FItemBase>(
		ItemID, TEXT("Item Lookup")
	);

	if (!Row)
		return false;

	OutData = *Row;
	return true;
}
