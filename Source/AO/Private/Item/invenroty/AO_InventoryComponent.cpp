#include "Item/invenroty/AO_InventoryComponent.h"

#include "AbilitySystemComponent.h"
#include "BlueprintEditor.h"
#include "EnhancedInputComponent.h"
#include "KismetTraceUtils.h"
#include "Character/AO_PlayerCharacter.h"
#include "Net/UnrealNetwork.h"
#include "GameFramework/Actor.h"
#include "Item/AO_MasterItem.h"
#include "Kismet/GameplayStatics.h"
#include "Navigation/PathFollowingComponent.h"

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

void UAO_InventoryComponent::SetupInputBinding(UInputComponent* PlayerInputComponent)
{
	if (!PlayerInputComponent)
	{
		return;
	}

	UEnhancedInputComponent* EIC = Cast<UEnhancedInputComponent>(PlayerInputComponent);
	if (!EIC)
	{
		return;
	}
	if (IA_Select_inventory_Slot)
	{
		EIC->BindAction(IA_Select_inventory_Slot, ETriggerEvent::Started, this, &UAO_InventoryComponent::SelectInventorySlot);
	}
	if (IA_UseItem)
	{
		EIC->BindAction(IA_UseItem, ETriggerEvent::Started, this, &UAO_InventoryComponent::OnLeftClick);
	}
	if (IA_DropItem)
	{
		EIC->BindAction(IA_DropItem, ETriggerEvent::Started, this, &UAO_InventoryComponent::OnRightClick);	
	}
}

void UAO_InventoryComponent::ServerSetSelectedSlot_Implementation(int32 NewIndex)
{
	if (!IsValidSlotIndex(NewIndex))
	{
		//UE_LOG(LogTemp, Warning, TEXT("SERVER: Invalid slot index %d"), NewIndex);
		return;
	}

	SelectedSlotIndex = NewIndex;
	
	if (OnInventoryUpdated.IsBound())
	{
		OnInventoryUpdated.Broadcast(Slots);
	}

	//UE_LOG(LogTemp, Verbose, TEXT("SERVER: SelectedSlotIndex set to %d"), SelectedSlotIndex);
}

void UAO_InventoryComponent::OnLeftClick()
{
	if (GetOwnerRole() < ROLE_Authority)
	{
		UseInventoryItem_Server();
	}
	else
	{
		UseInventoryItem_Server_Implementation();
	}
}

void UAO_InventoryComponent::OnRightClick()
{
	if (GetOwnerRole() < ROLE_Authority)
	{
		DropInventoryItem_Server();
	}
	else
	{
		DropInventoryItem_Server_Implementation();
	}
}

/*
아이템과 상호작용을함

현재 슬롯이 비어있는지 확인함.
y -> 인벤토리에 해당 아이템을 복사함
n -> 빈슬롯이 있는지 확인
	있다 -> 빈슬롯에 넣는다
	없다 -> 기존 아이템을 뱉어낸다
	*/

void UAO_InventoryComponent::PickupItem(const FInventorySlot& IncomingItem, AActor* Instigator)
{
	if (!IsValidSlotIndex(SelectedSlotIndex)) return;
	if (GetOwnerRole() != ROLE_Authority) return;

	AAO_MasterItem* WorldItemActor = Cast<AAO_MasterItem>(Instigator);
	if (!WorldItemActor) return;
	
	FInventorySlot OldSlot = Slots[SelectedSlotIndex];
	
	if (OldSlot.ItemID != "empty")
	{
		if (EmptySlotList.Num()>0)
		{
			int32 findEmptyNum = EmptySlotList[0];
			EmptySlotList.Remove(findEmptyNum);
			
			Slots[findEmptyNum] = IncomingItem;
			Slots[findEmptyNum].ItemType = IncomingItem.ItemType;
			Slots[findEmptyNum].FuelAmount = IncomingItem.FuelAmount;
			OnInventoryUpdated.Broadcast(Slots);
		}
		else
		{			
			Slots[SelectedSlotIndex] = IncomingItem;
			Slots[SelectedSlotIndex].ItemType = IncomingItem.ItemType;
			Slots[SelectedSlotIndex].FuelAmount = IncomingItem.FuelAmount;
			OnInventoryUpdated.Broadcast(Slots);

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
		Slots[SelectedSlotIndex].ItemType = IncomingItem.ItemType;
		Slots[SelectedSlotIndex].FuelAmount = IncomingItem.FuelAmount;
		OnInventoryUpdated.Broadcast(Slots);
		
		EmptySlotList.Remove(SelectedSlotIndex);
		
		WorldItemActor->Destroy();
	}
	
	FString DebugMessage;
	for (int32 Index : EmptySlotList)
	{
		DebugMessage += FString::Printf(TEXT("%d "), Index);
	}
	GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, DebugMessage);
}

void UAO_InventoryComponent::UseInventoryItem_Server_Implementation()
{
	if (Slots[SelectedSlotIndex].ItemType == EItemType::Consumable)
	{
		GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, "consume");
		
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
		GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, "Weapon");

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
			// 데미지 적용
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
	//GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, "DropItem");
	if (!IsValidSlotIndex(SelectedSlotIndex)) return;
	if (GetOwnerRole() != ROLE_Authority) return;
	
	const FInventorySlot& CurrentSlot = Slots[SelectedSlotIndex];

	if (CurrentSlot.ItemID != "empty")
	{
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
			DropItem->ItemID = CurrentSlot.ItemID;
			// DropItem->FuelAmount = CurrentSlot.FuelAmount;
			UGameplayStatics::FinishSpawningActor(DropItem, SpawnTransform);
		}
		ClearSlot();
	}
}

void UAO_InventoryComponent::OnRep_Slots()
{
	//UE_LOG(LogTemp, Verbose, TEXT("CLIENT: OnRep_Slots called (Owner: %s)"), *GetOwner()->GetName());
	
	if (OnInventoryUpdated.IsBound())
	{
		OnInventoryUpdated.Broadcast(Slots);
	}
}

void UAO_InventoryComponent::OnRep_SelectedIndex()
{
	/*UE_LOG(LogTemp, Verbose, TEXT("CLIENT: SelectedSlotIndex updated to: %d (Owner: %s)"),
		   SelectedSlotIndex,
		   *GetOwner()->GetName());*/
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
	
	if (OnInventoryUpdated.IsBound())
	{
		OnInventoryUpdated.Broadcast(Slots);
	}
	EmptySlotList.Add(SelectedSlotIndex);
}

//ms_inventory key binding
void UAO_InventoryComponent::SelectInventorySlot(const FInputActionValue& Value)
{
	//UE_LOG(LogTemp, Warning, TEXT("INPUT BINDING SUCCESS! Raw Slot Value (Float): %f"), Value.Get<float>());
	
	float SlotIndexAsFloat = Value.Get<float>();
	int32 SlotIndex = FMath::RoundToInt(SlotIndexAsFloat); 
	
	ServerSetSelectedSlot(SlotIndex);
	OnSelectSlotUpdated.Broadcast(SlotIndex);
}