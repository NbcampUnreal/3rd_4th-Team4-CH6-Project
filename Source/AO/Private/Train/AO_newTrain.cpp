#include "Train/AO_newTrain.h"
#include "AbilitySystemComponent.h"
#include "Game/GameMode/AO_GameMode_Stage.h" // JSH: 연료 실패 트리거
#include "Item/invenroty/AO_InventoryComponent.h"
#include "Train/AO_TrainWorldSubsystem.h"
#include "Train/ITrainFuelListener.h"

AAO_newTrain::AAO_newTrain()
{
	ASC = CreateDefaultSubobject<UAbilitySystemComponent>(TEXT("ASC"));
	ASC->SetIsReplicated(true);
	ASC->SetReplicationMode(EGameplayEffectReplicationMode::Minimal);
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
	}
	if (UWorld* World = GetWorld())
	{
		if (UAO_TrainWorldSubsystem* Subsystem =
			World->GetSubsystem<UAO_TrainWorldSubsystem>())
		{
			Subsystem->RegisterTrain(this);
			UE_LOG(LogTemp, Warning, TEXT("Train BeginPlay & RegisterTrain called"));

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

void AAO_newTrain::OnFuelChange(const FOnAttributeChangeData& Data)
{
	const float OldFuel = Data.OldValue;
	const float NewFuel = Data.NewValue;
	const float Delta = NewFuel - OldFuel;
	
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

	OnFuelChangedDelegate.Broadcast(NewFuel);
	
	//해당 로그에는 GI의 초기값은 적용되지 않음
	/*
	TotalFuelGained += Delta;
	if (Delta > 0.f)
	{
		UE_LOG(LogTemp, Warning, TEXT("🔥 연료 추가 +%.1f (누적합: %.1f)"), Delta, TotalFuelGained);
	}
	else if (Delta < 0.f)
	{
		UE_LOG(LogTemp, Warning, TEXT("💨 연료 감소 %.1f (누적합: %.1f)"), Delta, TotalFuelGained);
	}
	*/
	
}


void AAO_newTrain::HandleFuelAttributeChanged(const FOnAttributeChangeData& Data)
{
	const float NewFuel = Data.NewValue;
	
	for (auto& ListenerPtr : FuelListeners)
	{
		if (ListenerPtr.IsValid())
		{
			UObject* Listener = ListenerPtr.Get();
			ITrainFuelListener::Execute_OnFuelChanged(Listener, NewFuel);
			UE_LOG(LogTemp, Warning, TEXT("Fuel Changed: %f"), Data.NewValue);

		}
	}
}

void AAO_newTrain::BindFuel(UObject* Listener)
{
	if (!ASC || !Listener) return;

	if (!Listener->GetClass()->ImplementsInterface(UTrainFuelListener::StaticClass()))
	{
		return;
	}

	FuelListeners.AddUnique(Listener);
	
	const float CurrentFuel =
		ASC->GetNumericAttribute(UAO_Fuel_AttributeSet::GetFuelAttribute());

	ITrainFuelListener::Execute_OnFuelChanged(Listener, CurrentFuel);
	UE_LOG(LogTemp, Warning, TEXT("BindFuel: Initial Fuel = %f"), CurrentFuel);

}


void AAO_newTrain::BindFuelListener(UObject* Listener)
{
	if (!IsValid(Listener)) return;

	if (!Listener->GetClass()->ImplementsInterface(UTrainFuelListener::StaticClass()))
	{
		return;
	}

	BindFuel(Listener);
	UE_LOG(LogTemp, Warning, TEXT("BindFuelListener called"));

}
