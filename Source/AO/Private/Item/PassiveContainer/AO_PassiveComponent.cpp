#include "Item/PassiveContainer/AO_PassiveComponent.h"
#include "AbilitySystemComponent.h"

UAO_PassiveComponent::UAO_PassiveComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

void UAO_PassiveComponent::BeginPlay()
{
	Super::BeginPlay();

	AActor* OwnerActor = GetOwner();
	if (!OwnerActor) return;

	UAbilitySystemComponent* ASC = OwnerActor->FindComponentByClass<UAbilitySystemComponent>();
	if (!ASC) return;
	
	TArray<FGameplayTag> EventTags = {
		FGameplayTag::RequestGameplayTag(TEXT("Event.Interaction.AddPassive.MaxHP")),
		FGameplayTag::RequestGameplayTag(TEXT("Event.Interaction.AddPassive.Stamina")),
		FGameplayTag::RequestGameplayTag(TEXT("Event.Interaction.AddPassive.MoveSpeed"))
	};
	
	for (const FGameplayTag& Tag : EventTags)
	{
		ASC->GenericGameplayEventCallbacks.FindOrAdd(Tag)
			.AddUObject(this, &UAO_PassiveComponent::OnGameplayEventReceived);
	}
}

void UAO_PassiveComponent::OnGameplayEventReceived(const FGameplayEventData* Payload)
{
	if (!Payload)
	{
		return;
	}

	UAbilitySystemComponent* ASC = GetOwner()->FindComponentByClass<UAbilitySystemComponent>();
	if (!ASC)
	{
		return;
	}

	static FGameplayTag PassiveAmountTag = FGameplayTag::RequestGameplayTag(TEXT("Data.PassiveAmount"));
	FGameplayEffectContextHandle Context = ASC->MakeEffectContext();
	
	TSubclassOf<UGameplayEffect> SelectedPassive = MaxHpPassive;
	if (Payload->EventTag.ToString() == "Event.Interaction.AddPassive.Stamina")
	{
		SelectedPassive = MaxStaminaPassive;
	}
	else if (Payload->EventTag.ToString() == "Event.Interaction.AddPassive.MoveSpeed")
	{
		SelectedPassive = MovementPassive;
	}

	FGameplayEffectSpecHandle SpecHandle = ASC->MakeOutgoingSpec(SelectedPassive, 1.f, Context);

	if (!SelectedPassive)
	{
		return;
	}
	if (SpecHandle.IsValid())
	{
		SpecHandle.Data->SetSetByCallerMagnitude(PassiveAmountTag, Payload->EventMagnitude);
		ASC->ApplyGameplayEffectSpecToSelf(*SpecHandle.Data.Get());
	}
}
