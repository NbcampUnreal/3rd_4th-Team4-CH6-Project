#include "Train/AO_Train.h"
#include "AbilitySystemComponent.h"
#include "Game/GameMode/AO_GameMode_Stage.h" // JSH: 연료 실패 트리거
#include "Public/Item/inventory/AO_InventoryComponent.h"
#include "Train/GAS/AO_Fuel_AttributeSet.h"

AAO_Train::AAO_Train()
{
	PrimaryActorTick.bCanEverTick = false;
	bReplicates = true;
	
	StaticMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("StaticMesh"));
	SetRootComponent(StaticMesh);
	StaticMesh->SetIsReplicated(true);
	
	ASC = CreateDefaultSubobject<UAbilitySystemComponent>(TEXT("ASC"));
	ASC->SetIsReplicated(true);
	ASC->SetReplicationMode(EGameplayEffectReplicationMode::Minimal);
	FuelAttributeSet = CreateDefaultSubobject<UAO_Fuel_AttributeSet>(TEXT("AttributeSet"));

	InteractableComp = CreateDefaultSubobject<UAO_InteractableComponent>(TEXT("InteractableComponent"));
	if (InteractableComp)
	{
		InteractableComp->OnInteractionSuccess.AddDynamic(this, &AAO_Train::HandleInteractionSuccess);
	}
}

UAbilitySystemComponent* AAO_Train::GetAbilitySystemComponent() const
{
	return ASC;
}

void AAO_Train::BeginPlay()
{
	Super::BeginPlay();
		
	if (HasAuthority())
	{
		if (!ASC) return;

		ASC->InitAbilityActorInfo(this, this);

		if (HasAuthority())
		{
			if (AddEnergyAbilityClass)
			{
				ASC->GiveAbility(FGameplayAbilitySpec(AddEnergyAbilityClass, 1, 0, this));
			}
			if (LeakEnergyAbilityClass)
			{
				ASC->GiveAbility(FGameplayAbilitySpec(LeakEnergyAbilityClass, 1, 0, this));
			}
		}
	}
	
	ASC->GetGameplayAttributeValueChangeDelegate(
			UAO_Fuel_AttributeSet::GetFuelAttribute()
		).AddUObject(this, &AAO_Train::OnFuelChanged);
}

void AAO_Train::OnFuelChanged(const FOnAttributeChangeData& Data)
{
	const float OldFuel = Data.OldValue;
	const float NewFuel = Data.NewValue;
	const float Delta = NewFuel - OldFuel;

	TotalFuelGained += Delta;

	if (HasAuthority())
	{
		// 연료가 0 이상이었다가 0 미만으로 떨어지는 순간에만 실패 트리거
		if (OldFuel >= 0.0f && NewFuel < 0.0f)
		{
			if (UWorld* World = GetWorld())
			{
				if (AAO_GameMode_Stage* StageGM = World->GetAuthGameMode<AAO_GameMode_Stage>())
				{
					StageGM->TriggerStageFailByTrainFuel();
				}
			}
		}
	}

	
	if (Delta > 0.f)
	{
		UE_LOG(LogTemp, Warning, TEXT("🔥 연료 추가 +%.1f (누적합: %.1f)"), Delta, TotalFuelGained);
	}
	else if (Delta < 0.f)
	{
		UE_LOG(LogTemp, Warning, TEXT("💨 연료 감소 %.1f (누적합: %.1f)"), Delta, TotalFuelGained);
	}
	

	OnFuelChangedDelegate.Broadcast(NewFuel);
}

void AAO_Train::FuelLeakSkillOn()
{
	if (!LeakEnergyAbilityClass) return;

	FGameplayAbilitySpec* Spec = ASC->FindAbilitySpecFromClass(LeakEnergyAbilityClass);

	if (Spec)
	{
		ASC->TryActivateAbility(Spec->Handle);
	}
}

void AAO_Train::FuelLeakSkillOut()
{
	if (!LeakEnergyAbilityClass) return;

	FGameplayAbilitySpec* Spec = ASC->FindAbilitySpecFromClass(LeakEnergyAbilityClass);
	
	ASC->CancelAbility(Spec->Ability);
	ASC->ClearAbility(Spec->Handle);
}

void AAO_Train::HandleInteractionSuccess(AActor* Interactor)
{
	if (!HasAuthority() || !ASC) 
	{
		return;
	}

	UAO_InventoryComponent* Inventory = Interactor->FindComponentByClass<UAO_InventoryComponent>();
	if (!Inventory) return;

	if (!Inventory->Slots.IsValidIndex(Inventory->SelectedSlotIndex))
	{
		return;
	}

	FInventorySlot& Slot = Inventory->Slots[Inventory->SelectedSlotIndex];

	float FuelFromItem = Slot.FuelAmount;

	if (FuelFromItem <= 0.f)
	{
		return;
	}
	
	const FGameplayTag ActivationEventTag = FGameplayTag::RequestGameplayTag(TEXT("Event.Interaction.AddFuel")); 
    
	FGameplayEventData EventData;
	EventData.EventTag = ActivationEventTag;
	EventData.Target = this;
	EventData.Instigator = Interactor;
	EventData.EventMagnitude = FuelFromItem;
	
	ASC->HandleGameplayEvent(
	   ActivationEventTag, 
	   &EventData
	);
	
	Inventory->ClearSlot();	
}