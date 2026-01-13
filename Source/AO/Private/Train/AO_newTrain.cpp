#include "Train/AO_newTrain.h"
#include "AbilitySystemComponent.h"
#include "Game/GameMode/AO_GameMode_Stage.h" // JSH: 연료 실패 트리거
#include "Public/Item/inventory/AO_InventoryComponent.h"
#include "Train/AO_TrainWorldSubsystem.h"
#include "Train/AO_TrainFuelListener.h"

AAO_newTrain::AAO_newTrain()
{
	ASC = CreateDefaultSubobject<UAbilitySystemComponent>(TEXT("ASC"));
	ASC->SetIsReplicated(true);
	ASC->SetReplicationMode(EGameplayEffectReplicationMode::Minimal);

	InteractionTitle = FText::FromString(TEXT("[F]"));
	InteractionContent = FText::FromString(TEXT("Fuel Insert"));
}

void AAO_newTrain::BeginPlay()
{
	Super::BeginPlay();
	
	check(ASC);
	ASC->InitAbilityActorInfo(this, this);
	
	FuelAttributeSet = ASC->GetSet<UAO_Fuel_AttributeSet>();
	check(FuelAttributeSet);
		
	ASC->GetGameplayAttributeValueChangeDelegate(
		UAO_Fuel_AttributeSet::GetFuelAttribute()
	).AddUObject(this, &AAO_newTrain::HandleFuelAttributeChanged);
	
	if (HasAuthority())
	{
		const_cast<UAO_Fuel_AttributeSet*>(FuelAttributeSet)->InitFromGameInstance();
	
		if (AddEnergyAbilityClass)
		{
			ASC->GiveAbility(FGameplayAbilitySpec(AddEnergyAbilityClass, 1, 0));
		}
		if (LeakEnergyAbilityClass)
		{
			ASC->GiveAbility(FGameplayAbilitySpec(LeakEnergyAbilityClass, 1, 0));
		}
		FuelLeakSkillOn();
	}
	if (UWorld* World = GetWorld())
	{
		if (UAO_TrainWorldSubsystem* Subsystem =
			World->GetSubsystem<UAO_TrainWorldSubsystem>())
		{
			Subsystem->RegisterTrain(this);
		}
	}
}

UAbilitySystemComponent* AAO_newTrain::GetAbilitySystemComponent() const
{
	return ASC;
}

void AAO_newTrain::FuelLeakSkillOn()
{
	if (!LeakEnergyAbilityClass) return;

	FGameplayAbilitySpec* Spec = ASC->FindAbilitySpecFromClass(LeakEnergyAbilityClass);

	if (Spec)
	{
		ASC->TryActivateAbility(Spec->Handle);
	}
}

void AAO_newTrain::FuelLeakSkillOut()
{
	if (!LeakEnergyAbilityClass) return;

	FGameplayAbilitySpec* Spec = ASC->FindAbilitySpecFromClass(LeakEnergyAbilityClass);
	
	ASC->CancelAbility(Spec->Ability);
	ASC->ClearAbility(Spec->Handle);
}

void AAO_newTrain::OnInteractionSuccess(AActor* Interactor)
{
	Super::OnInteractionSuccess(Interactor);
	
	if (!HasAuthority() || !ASC) 
	{
		return;
	}

	UAO_InventoryComponent* Inventory = Interactor->FindComponentByClass<UAO_InventoryComponent>();
	
	if (!Inventory) return;
	if (!Inventory->Slots.IsValidIndex(Inventory->SelectedSlotIndex)) return;
	
	FInventorySlot& Slot = Inventory->Slots[Inventory->SelectedSlotIndex];

	float FuelFromItem = Slot.FuelAmount;

	if (FuelFromItem <= 0.f) return;
	
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

void AAO_newTrain::HandleFuelAttributeChanged(const FOnAttributeChangeData& Data)
{
	const float OldFuel = Data.OldValue;
	const float NewFuel = Data.NewValue;
	
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
	for (auto& ListenerPtr : FuelListeners)
	{
		if (ListenerPtr.IsValid())
		{
			UObject* Listener = ListenerPtr.Get();
			IAO_TrainFuelListener::Execute_OnFuelChanged(Listener, NewFuel);
		}
	}
}

void AAO_newTrain::BindFuel(UObject* Listener)
{
	if (!ASC || !Listener) return;

	if (!Listener->GetClass()->ImplementsInterface(UAO_TrainFuelListener::StaticClass()))	return;

	FuelListeners.AddUnique(Listener);
	
	const float CurrentFuel =
		ASC->GetNumericAttribute(UAO_Fuel_AttributeSet::GetFuelAttribute());

	IAO_TrainFuelListener::Execute_OnFuelChanged(Listener, CurrentFuel);
}

void AAO_newTrain::BindFuelListener(UObject* Listener)
{
	if (!IsValid(Listener)) return;

	if (!Listener->GetClass()->ImplementsInterface(UAO_TrainFuelListener::StaticClass()))	return;

	BindFuel(Listener);
}