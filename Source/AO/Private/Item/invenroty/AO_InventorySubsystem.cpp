#include "Item/invenroty/AO_InventorySubsystem.h"
#include "AO_Log.h"
#include "GameFramework/PlayerController.h"
#include "GameFramework/Controller.h"
#include "Item/invenroty/AO_InventoryComponent.h"

void UAO_InventorySubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

	if (ULocalPlayer* LP = GetLocalPlayer())
	{
		if (APlayerController* PC = LP->GetPlayerController(nullptr))
		{
			if (!PC->IsLocalController())
				return;

			PC->OnPossessedPawnChanged.AddDynamic(
				this,
				&UAO_InventorySubsystem::HandlePossessedPawnChanged
			);
		}
	}
}

void UAO_InventorySubsystem::HandlePossessedPawnChanged(APawn* OldPawn, APawn* NewPawn)
{
	CachedInvenComp = nullptr;
	if (!NewPawn) return;

	if (UAO_InventoryComponent* Comp = NewPawn->FindComponentByClass<UAO_InventoryComponent>())
	{
		Comp->RegisterToSubsystem();
		RegisterInventory(Comp);
	}
}

void UAO_InventorySubsystem::RegisterInventory(UAO_InventoryComponent* InvenComp)
{
	if (!InvenComp) return;

	CachedInvenComp = InvenComp;
	OnInvenRegistered.Broadcast(InvenComp);
}

UAO_InventoryComponent* UAO_InventorySubsystem::GetInvenComp() const
{
	return CachedInvenComp.Get();
}
