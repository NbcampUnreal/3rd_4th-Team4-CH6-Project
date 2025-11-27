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
	
	FGameplayTag EventTag = FGameplayTag::RequestGameplayTag(TEXT("Event.Interaction.AddPassive"));
	
	/*
	ASC->GenericGameplayEventCallbacks.FindOrAdd(EventTag).AddUObject(this, &UAO_PassiveComponent::OnGameplayEventReceived);
	*/
}

/*
void UAO_PassiveComponent::OnGameplayEventReceived(const FGameplayEventData* Payload)
{
	if (!Payload || !PassiveEffectClass) return;

	UAbilitySystemComponent* ASC = Cast<UAbilitySystemComponent>(Payload->Target->FindComponentByClass<UAbilitySystemComponent>());
	if (!ASC) return;

	FGameplayEffectContextHandle EffectContext = ASC->MakeEffectContext();
	EffectContext.AddSourceObject(this);

	FGameplayEffectSpecHandle SpecHandle = ASC->MakeOutgoingSpec(PassiveEffectClass, 1.f, EffectContext);
	if (SpecHandle.IsValid())
	{
		SpecHandle.Data->SetSetByCallerMagnitude(
			FGameplayTag::RequestGameplayTag(TEXT("Data.PassiveAmount")),
			Payload->EventMagnitude
		);

		ASC->ApplyGameplayEffectSpecToSelf(*SpecHandle.Data.Get());
	}
}
*/
