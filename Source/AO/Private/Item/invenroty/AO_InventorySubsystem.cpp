#include "Item/invenroty/AO_InventorySubsystem.h"

#include "AO_Log.h"
#include "ToolWidgetsSlateTypes.h"
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
			UE_LOG(LogJSH, Warning,
	   TEXT("InventorySubsystem Init: PC=%s HasAuthority=%d IsLocal=%d WorldType=%d"),
	   *PC->GetName(),
	   PC->HasAuthority(),
	   PC->IsLocalController(),
	   (int32)GetWorld()->WorldType);

			PC->OnPossessedPawnChanged.AddDynamic(
				this,
				&UAO_InventorySubsystem::HandlePossessedPawnChanged
			);
			
			if (!PC->IsLocalController())
			{
				UE_LOG(LogJSH, Warning,
					TEXT("InventorySubsystem: Skip non-local PC %s"), *PC->GetName());
				return;
			}

			PC->OnPossessedPawnChanged.AddDynamic(
				this,
				&UAO_InventorySubsystem::HandlePossessedPawnChanged
			);
		}
		else
		{
			UE_LOG(LogJSH, Warning, TEXT("InventorySubsystem: PlayerController not found"));
		}
	}
	else
	{
		UE_LOG(LogJSH, Warning, TEXT("InventorySubsystem: LocalPlayer not found"));
	}
}


void UAO_InventorySubsystem::HandlePossessedPawnChanged(APawn* OldPawn, APawn* NewPawn)
{
	UE_LOG(LogJSH, Warning,
		TEXT("HandlePossessedPawnChanged: Old=%s New=%s WorldType=%d"),
		OldPawn ? *OldPawn->GetName() : TEXT("None"),
		NewPawn ? *NewPawn->GetName() : TEXT("None"),
		(int32)GetWorld()->WorldType);
	
	CachedInvenComp = nullptr;

	if (!NewPawn) return;

	if (UAO_InventoryComponent* Comp =
	NewPawn->FindComponentByClass<UAO_InventoryComponent>())
	{
		Comp->RegisterToSubsystem();   // 컴포넌트 → 서브시스템
		RegisterInventory(Comp);       // 서브시스템 → UI Broadcast
	}
	else
	{
		UE_LOG(LogJSH, Warning, TEXT("InventorySubsystem: InventoryComponent not found"));
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
