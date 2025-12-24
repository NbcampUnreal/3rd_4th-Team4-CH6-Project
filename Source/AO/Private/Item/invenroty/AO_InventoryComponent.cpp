#include "Item/invenroty/AO_InventoryComponent.h"
#include "AbilitySystemComponent.h"
#include "Item/invenroty/AO_InventorySubsystem.h"
#include "GameFramework/Pawn.h"
#include "EnhancedInputComponent.h"
#include "Net/UnrealNetwork.h"
#include "Item/AO_MasterItem.h"
#include "Item/invenroty/AO_InventoryListener.h"
#include "Kismet/GameplayStatics.h"

UAO_InventoryComponent::UAO_InventoryComponent()
{
    PrimaryComponentTick.bCanEverTick = false;
    SetIsReplicatedByDefault(true);
    
    Slots.SetNum(5);
    for (FInventorySlot& Slot : Slots)
    {
       Slot = FInventorySlot();
    }

    SelectedSlotIndex = 1;
    EmptySlotList = {1, 2, 3, 4};
}

void UAO_InventoryComponent::BeginPlay()
{
	Super::BeginPlay();
		
	APawn* OwnerPawn = Cast<APawn>(GetOwner());
	if (!OwnerPawn) return;

	APlayerController* PC = Cast<APlayerController>(OwnerPawn->GetController());
	if (!PC || !PC->IsLocalController())
		return;

	if (ULocalPlayer* LP = PC->GetLocalPlayer())
	{
		if (auto* Subsystem = LP->GetSubsystem<UAO_InventorySubsystem>())
		{
			Subsystem->RegisterInventory(this);
		}
	}
}

void UAO_InventoryComponent::RegisterToSubsystem()
{
	APawn* Pawn = Cast<APawn>(GetOwner());
	if (!Pawn) return;

	APlayerController* PC = Cast<APlayerController>(Pawn->GetController());
	if (!PC || !PC->IsLocalController())
		return;

	if (ULocalPlayer* LP = PC->GetLocalPlayer())
	{
		if (auto* Subsystem = LP->GetSubsystem<UAO_InventorySubsystem>())
		{
			UE_LOG(LogTemp, Warning, TEXT("Register Inventory to Subsystem"));
			Subsystem->RegisterInventory(this);
		}
		else
		{
			UE_LOG(LogTemp, Warning, TEXT("Subsystem is null"));
		}
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("LocalPlayer is null"));
	}
}

void UAO_InventoryComponent::SetupInputBinding(UInputComponent* PlayerInputComponent)
{
    if (!PlayerInputComponent) return;
    UEnhancedInputComponent* EIC = Cast<UEnhancedInputComponent>(PlayerInputComponent);
    if (!EIC) return;

    if (IA_Select_inventory_Slot) EIC->BindAction(IA_Select_inventory_Slot, ETriggerEvent::Started, this, &UAO_InventoryComponent::SelectInventorySlot);
    if (IA_UseItem) EIC->BindAction(IA_UseItem, ETriggerEvent::Started, this, &UAO_InventoryComponent::OnLeftClick);
    if (IA_DropItem) EIC->BindAction(IA_DropItem, ETriggerEvent::Started, this, &UAO_InventoryComponent::OnRightClick); 
}

void UAO_InventoryComponent::ServerSetSelectedSlot_Implementation(int32 NewIndex)
{
    if (!IsValidSlotIndex(NewIndex)) return;
    SelectedSlotIndex = NewIndex;
    
    NotifyListeners();
    if (OnInventoryUpdated.IsBound()) OnInventoryUpdated.Broadcast(Slots);
}

void UAO_InventoryComponent::OnLeftClick()
{
    if (GetOwnerRole() < ROLE_Authority) UseInventoryItem_Server();
    else UseInventoryItem_Server_Implementation();
}

void UAO_InventoryComponent::OnRightClick()
{
    if (GetOwnerRole() < ROLE_Authority) DropInventoryItem_Server();
    else DropInventoryItem_Server_Implementation();
}

void UAO_InventoryComponent::PickupItem(const FInventorySlot& IncomingItem, AActor* Instigator)
{
    if (!IsValidSlotIndex(SelectedSlotIndex) || GetOwnerRole() != ROLE_Authority) return;
    
    FInventorySlot OldSlot = Slots[SelectedSlotIndex];
    if (OldSlot.ItemID != "empty")
    {
       if (EmptySlotList.Num() > 0)
       {
          int32 findEmptyNum = EmptySlotList[0];
          EmptySlotList.Remove(findEmptyNum);
          Slots[findEmptyNum] = IncomingItem;
       }
       else
       {        
          Slots[SelectedSlotIndex] = IncomingItem;
          EmptySlotList.Remove(SelectedSlotIndex);
          
       	FVector SpawnLocation = GetOwner()->GetActorLocation() + GetOwner()->GetActorForwardVector() * 40.f;
       	FRotator SpawnRotation = FRotator::ZeroRotator;
       	FTransform SpawnTransform(SpawnRotation, SpawnLocation);

       	AAO_MasterItem* DropItem = GetWorld()->SpawnActorDeferred<AAO_MasterItem>(
			   DroppableItemClass ? DroppableItemClass.Get() : AAO_MasterItem::StaticClass(),
			   SpawnTransform,
			   nullptr,
			   nullptr,
			   ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn
		   );

       	if (DropItem)
       	{
       		DropItem->ItemID = OldSlot.ItemID;
       		UGameplayStatics::FinishSpawningActor(DropItem, SpawnTransform);
       	}
       }
    }
    else
    {
       Slots[SelectedSlotIndex] = IncomingItem;
       EmptySlotList.Remove(SelectedSlotIndex);
       if (Instigator) Instigator->Destroy();
    }
    
    NotifyListeners();
    OnInventoryUpdated.Broadcast(Slots);
}

void UAO_InventoryComponent::UseInventoryItem_Server_Implementation()
{
	if (Slots[SelectedSlotIndex].ItemType == EItemType::Consumable)
	{
		static const FString Context00(TEXT("Inventory Item Use"));
		float AddAmount = 0.0f;
		const FAO_struct_FItemBase* ItemData = ItemDataTable->FindRow<FAO_struct_FItemBase>(
		Slots[SelectedSlotIndex].ItemID, 
		Context00      
		);

		if (ItemData)
		{
			AddAmount = ItemData->PassiveAmount;
		}
		const FGameplayTag ActivationEventTag = FGameplayTag::RequestGameplayTag(TEXT("Event.Interaction.AddPassive")); 
    
		FGameplayEventData EventData;
		EventData.EventTag = ActivationEventTag;
		EventData.EventMagnitude = AddAmount;
		
		AActor* OwnerActor = GetOwner();
		if (!OwnerActor) return;
		UAbilitySystemComponent* ASC = OwnerActor->FindComponentByClass<UAbilitySystemComponent>();
		if (!ASC) return;
		static FGameplayTag PassiveAmountTag = FGameplayTag::RequestGameplayTag(TEXT("Data.Item"));    
		FGameplayEffectContextHandle Context = ASC->MakeEffectContext();  
		FGameplayEffectSpecHandle SpecHandle = ASC->MakeOutgoingSpec(AddHealthClass, 1.f, Context);    
		if (SpecHandle.IsValid())  
		{
			SpecHandle.Data->SetSetByCallerMagnitude(PassiveAmountTag, EventData.EventMagnitude);  
			ASC->ApplyGameplayEffectSpecToSelf(*SpecHandle.Data.Get());
		}
		
		ClearSlot();
	}
	if (Slots[SelectedSlotIndex].ItemType == EItemType::Weapon)
	{
		//GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, "Weapon");

		FHitResult Hit;
		FVector Start = GetOwner()->GetActorLocation();
		FVector End = Start + (GetOwner()->GetActorForwardVector() * 10000.f);

		FCollisionQueryParams Params;
		Params.AddIgnoredActor(GetOwner());

		bool bHit = GetWorld()->LineTraceSingleByChannel(
			Hit,
			Start,
			End,
			ECC_Visibility,
			Params
		);

		if (bHit)
		{
			if (AActor* HitActor = Hit.GetActor())
			{
				UGameplayStatics::ApplyPointDamage(
					HitActor,
					1.0f,
					(End - Start).GetSafeNormal(),
					Hit,
					GetOwner()->GetInstigatorController(),
			GetOwner(),   
					UDamageType::StaticClass()
				);
			}
			DrawDebugLine(GetWorld(), Start, End, FColor::Red, false, 1.f, 0, 1.f);

			// effect
		}

	}
}

void UAO_InventoryComponent::DropInventoryItem_Server_Implementation()
{
	if (!IsValidSlotIndex(SelectedSlotIndex)) return;
	if (GetOwnerRole() != ROLE_Authority) return;
	
	const FInventorySlot& CurrentSlot = Slots[SelectedSlotIndex];

	if (CurrentSlot.ItemID != "empty")
	{
		FVector SpawnLocation = GetOwner()->GetActorLocation() + GetOwner()->GetActorForwardVector() * 50.f;
		FRotator SpawnRotation = FRotator::ZeroRotator;
		FTransform SpawnTransform(SpawnRotation, SpawnLocation);
		
		 FActorSpawnParameters Params;
		Params.SpawnCollisionHandlingOverride =
		ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;

		Params.Owner = GetOwner();

		AAO_MasterItem* DropItem =
	GetWorld()->SpawnActorDeferred<AAO_MasterItem>(
		DroppableItemClass,
		SpawnTransform,
		GetOwner(),
		nullptr,
		ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn
	);

		if (DropItem)
		{
			DropItem->ItemID = CurrentSlot.ItemID;
			UGameplayStatics::FinishSpawningActor(DropItem, SpawnTransform);
		
			// DropItem->FuelAmount = CurrentSlot.FuelAmount;
		}
		ClearSlot();
	}
}

void UAO_InventoryComponent::OnRep_Slots()
{
    NotifyListeners();
    if (OnInventoryUpdated.IsBound()) OnInventoryUpdated.Broadcast(Slots);
}

void UAO_InventoryComponent::OnRep_SelectedIndex()
{
    NotifyListeners();
}

void UAO_InventoryComponent::BindInvenCompListener(UObject* Listener)
{
    if (!IsValid(Listener) || !Listener->GetClass()->ImplementsInterface(UAO_InventoryListener::StaticClass())) return;

    InvenCompListeners.AddUnique(Listener);
	
    IAO_InventoryListener::Execute_OnSelectChanged(Listener, SelectedSlotIndex);
    IAO_InventoryListener::Execute_OnSlotChanged(Listener, Slots);
}

void UAO_InventoryComponent::NotifyListeners()
{
    for (int32 i = InvenCompListeners.Num() - 1; i >= 0; --i)
    {
        if (InvenCompListeners[i].IsValid())
        {
            IAO_InventoryListener::Execute_OnSelectChanged(InvenCompListeners[i].Get(), SelectedSlotIndex);
            IAO_InventoryListener::Execute_OnSlotChanged(InvenCompListeners[i].Get(), Slots);
        }
        else
        {
            InvenCompListeners.RemoveAt(i);
        }
    }
}

void UAO_InventoryComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);
    DOREPLIFETIME(UAO_InventoryComponent, Slots);
    DOREPLIFETIME(UAO_InventoryComponent, SelectedSlotIndex);
}

void UAO_InventoryComponent::ClearSlot()
{
    Slots[SelectedSlotIndex] = FInventorySlot();
    EmptySlotList.Add(SelectedSlotIndex);
    NotifyListeners();
    if (OnInventoryUpdated.IsBound()) OnInventoryUpdated.Broadcast(Slots);
}

void UAO_InventoryComponent::SelectInventorySlot(const FInputActionValue& Value)
{
    int32 SlotIndex = FMath::RoundToInt(Value.Get<float>()); 
    ServerSetSelectedSlot(SlotIndex);
    OnSelectSlotUpdated.Broadcast(SlotIndex);
}