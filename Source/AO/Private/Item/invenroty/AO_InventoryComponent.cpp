#include "Public/Item/inventory/AO_InventoryComponent.h"
#include "AbilitySystemComponent.h"
#include "AbilitySystemInterface.h"
#include "Public/Item/inventory/AO_InventorySubsystem.h"
#include "GameFramework/Pawn.h"
#include "EnhancedInputComponent.h"
#include "GameFramework/Character.h"
#include "Net/UnrealNetwork.h"
#include "Item/AO_MasterItem.h"
#include "Public/Item/inventory/AO_InventoryListener.h"
#include "Kismet/GameplayStatics.h"
#include "Player/PlayerState/AO_PlayerState.h"

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
    bDroppedOnDeath = false;
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
          Subsystem->RegisterInventory(this);
       }
    }
}

void UAO_InventoryComponent::SetupInputBinding(UInputComponent* PlayerInputComponent)
{
    if (Slots.Num() == 0) return;

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
    UseInventoryItem_Server();
}

void UAO_InventoryComponent::OnRightClick()
{
    DropInventoryItem_Server();
}

int32 UAO_InventoryComponent::FindEmptySlotIndex() const
{
    for (int32 i = 1; i < Slots.Num(); ++i)
    {
        if (Slots[i].ItemID == "empty")
        {
            return i;
        }
    }
    return INDEX_NONE;
}

void UAO_InventoryComponent::PickupItem(const FInventorySlot& IncomingItem, AActor* Instigator)
{
    if (GetOwnerRole() != ROLE_Authority) return;
    
    int32 TargetIndex = FindEmptySlotIndex();
    bool bPickupSuccess = false;

    if (TargetIndex != INDEX_NONE)
    {
        Slots[TargetIndex] = IncomingItem;
        bPickupSuccess = true;
    }
    else if (IsValidSlotIndex(SelectedSlotIndex))
    {
        FInventorySlot OldSlot = Slots[SelectedSlotIndex];
        Slots[SelectedSlotIndex] = IncomingItem;
        
        FVector SpawnLocation = GetOwner()->GetActorLocation() + GetOwner()->GetActorForwardVector() * 60.f;
        FTransform SpawnTransform(FRotator::ZeroRotator, SpawnLocation);

        AAO_MasterItem* DropItem = GetWorld()->SpawnActorDeferred<AAO_MasterItem>(
             DroppableItemClass ? DroppableItemClass.Get() : AAO_MasterItem::StaticClass(),
             SpawnTransform,
             GetOwner(),
             nullptr,
             ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn
        );
        bPickupSuccess = true;
    }

    if (bPickupSuccess && Instigator)
    {
        if (IAbilitySystemInterface* ASI = Cast<IAbilitySystemInterface>(Instigator))
        {
            if (UAbilitySystemComponent* ASC = ASI->GetAbilitySystemComponent())
            {
                //시체 ASC에서 death ability 종료시키는 로직
                TArray<FGameplayAbilitySpecHandle> AllAbilities;
                for (const FGameplayAbilitySpec& Spec : ASC->GetActivatableAbilities())
                {
                    AllAbilities.Add(Spec.Handle);
                }

                for (const FGameplayAbilitySpecHandle& Handle : AllAbilities)
                {
                    ASC->CancelAbilityHandle(Handle);
                    ASC->ClearAbility(Handle);
                }
            }
        }
        Instigator->SetActorHiddenInGame(true);
        Instigator->SetActorEnableCollision(false);
        Instigator->SetLifeSpan(0.1f); 
    }
    
    NotifyListeners();
    OnInventoryUpdated.Broadcast(Slots);
}

void UAO_InventoryComponent::UseInventoryItem_Server_Implementation()
{
    if (!IsValidSlotIndex(SelectedSlotIndex)) return;
    if (Slots[SelectedSlotIndex].ItemID == "empty") return;

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
       
       AActor* OwnerActor = GetOwner();
       UAbilitySystemComponent* ASC = OwnerActor->FindComponentByClass<UAbilitySystemComponent>();
       
       if (ASC)
       {
           static FGameplayTag PassiveAmountTag = FGameplayTag::RequestGameplayTag(TEXT("Data.Item"));    
           FGameplayEffectContextHandle Context = ASC->MakeEffectContext();  
           FGameplayEffectSpecHandle SpecHandle = ASC->MakeOutgoingSpec(AddHealthClass, 1.f, Context);    
           if (SpecHandle.IsValid())  
           {
              SpecHandle.Data->SetSetByCallerMagnitude(PassiveAmountTag, AddAmount);  
              ASC->ApplyGameplayEffectSpecToSelf(*SpecHandle.Data.Get());
           }
       }
       
       ClearSlot();
    }
    else if (Slots[SelectedSlotIndex].ItemType == EItemType::Weapon)
    {
       FHitResult Hit;
       FVector Start = GetOwner()->GetActorLocation();
       FVector End = Start + (GetOwner()->GetActorForwardVector() * 10000.f);

       FCollisionQueryParams Params;
       Params.AddIgnoredActor(GetOwner());

       if (GetWorld()->LineTraceSingleByChannel(Hit, Start, End, ECC_Visibility, Params))
       {
          if (AActor* HitActor = Hit.GetActor())
          {
             UGameplayStatics::ApplyPointDamage(HitActor, 1.0f, (End - Start).GetSafeNormal(), Hit, GetOwner()->GetInstigatorController(), GetOwner(), UDamageType::StaticClass());
          }
          DrawDebugLine(GetWorld(), Start, End, FColor::Red, false, 1.f, 0, 1.f);
       }
    }
}

void UAO_InventoryComponent::DropInventoryItem_Server_Implementation()
{
    if (!IsValidSlotIndex(SelectedSlotIndex)) return;
    
    const FInventorySlot& CurrentSlot = Slots[SelectedSlotIndex];

    if (CurrentSlot.ItemID != "empty")
    {
       FVector SpawnLocation = GetOwner()->GetActorLocation() + GetOwner()->GetActorForwardVector() * 50.f;
       FTransform SpawnTransform(FRotator::ZeroRotator, SpawnLocation);
       
       AAO_MasterItem* DropItem = GetWorld()->SpawnActorDeferred<AAO_MasterItem>(
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

void UAO_InventoryComponent::UnBindInvenCompListener(UObject* Listener)
{
    if (!IsValid(Listener) || !Listener->GetClass()->ImplementsInterface(UAO_InventoryListener::StaticClass())) return;

    InvenCompListeners.Remove(Listener);
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
    if (IsValidSlotIndex(SelectedSlotIndex))
    {
        Slots[SelectedSlotIndex] = FInventorySlot();
        NotifyListeners();
        if (OnInventoryUpdated.IsBound()) OnInventoryUpdated.Broadcast(Slots);
    }
}

void UAO_InventoryComponent::SelectInventorySlot(const FInputActionValue& Value)
{
    int32 SlotIndex = FMath::RoundToInt(Value.Get<float>()); 
    if (IsValidSlotIndex(SlotIndex))
    {
        ServerSetSelectedSlot(SlotIndex);
        OnSelectSlotUpdated.Broadcast(SlotIndex);
    }
}

void UAO_InventoryComponent::CharDead()
{
    if (bDroppedOnDeath || GetOwnerRole() != ROLE_Authority) return;

    bDroppedOnDeath = true;
    
    for (int32 i = 0; i < Slots.Num(); i++)
    {
       if (Slots[i].ItemID == "empty") continue;
       
       FVector SpawnLocation = GetOwner()->GetActorLocation() + GetOwner()->GetActorForwardVector() * 50.f + FVector(0.f, i * 20.f, 0.f);
       FTransform SpawnTransform(FRotator::ZeroRotator, SpawnLocation);

       AAO_MasterItem* DropItem = GetWorld()->SpawnActorDeferred<AAO_MasterItem>(
             DroppableItemClass,
             SpawnTransform,
             GetOwner(),
             nullptr,
             ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn
       );

       if (DropItem)
       {
          DropItem->ItemID = Slots[i].ItemID;
          UGameplayStatics::FinishSpawningActor(DropItem, SpawnTransform);
          Slots[i] = FInventorySlot();
       }
    }
    NotifyListeners();
}

void UAO_InventoryComponent::StoreToPlayerState()
{
    if (GetOwnerRole() != ROLE_Authority) return;

    if (ACharacter* Character = Cast<ACharacter>(GetOwner()))
    {
       if (AAO_PlayerState* AO_PS = Character->GetPlayerState<AAO_PlayerState>())
       {
          AO_PS->PersistentInventory = Slots;
       }
    }
}

void UAO_InventoryComponent::ApplySlotsFromSave(const TArray<FInventorySlot>& NewSlots)
{
    if (NewSlots.Num() == 0) return;

    Slots = NewSlots;

    NotifyListeners();
    if (OnInventoryUpdated.IsBound()) OnInventoryUpdated.Broadcast(Slots);
}

bool UAO_InventoryComponent::CanInventoryAction() const
{
	if (TObjectPtr<AActor> Owner = GetOwner())
	{
		if (TObjectPtr<UAbilitySystemComponent> ASC = Owner->FindComponentByClass<UAbilitySystemComponent>())
		{
			// Inspection 중이면 인벤토리 액션 불가
			if (ASC->HasMatchingGameplayTag(FGameplayTag::RequestGameplayTag(FName("Status.Action.Inspecting"))))
			{
				return false;
			}
		}
	}
	return true;
}
