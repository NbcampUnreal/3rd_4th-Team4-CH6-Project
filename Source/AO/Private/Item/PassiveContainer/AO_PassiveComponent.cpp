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
	
	ASC->GenericGameplayEventCallbacks.FindOrAdd(EventTag)
	.AddUObject(this, &UAO_PassiveComponent::OnGameplayEventReceived);
}

void UAO_PassiveComponent::OnGameplayEventReceived(const FGameplayEventData* Payload)
{
	UE_LOG(LogTemp, Warning, TEXT("OnGameplayEventReceived() Triggered"));

	if (!Payload || !PassiveEffectClass)
	{
		UE_LOG(LogTemp, Warning, TEXT("Payload NULL or PassiveEffectClass NOT set"));
		return;
	}

	UAbilitySystemComponent* ASC = GetOwner()->FindComponentByClass<UAbilitySystemComponent>();
	if (!ASC)
	{
		UE_LOG(LogTemp, Warning, TEXT("ASC NOT Found on Owner"));
		return;
	}

	static FGameplayTag PassiveAmountTag = FGameplayTag::RequestGameplayTag(TEXT("Data.PassiveAmount"));

	UE_LOG(LogTemp, Warning, TEXT("Passive Event Received: EventMagnitude = %f"), Payload->EventMagnitude);
	UE_LOG(LogTemp, Warning, TEXT("Using GE Class: %s"), *PassiveEffectClass->GetName());

	FGameplayEffectContextHandle Context = ASC->MakeEffectContext();
	FGameplayEffectSpecHandle SpecHandle = ASC->MakeOutgoingSpec(PassiveEffectClass, 1.f, Context);

	if (SpecHandle.IsValid())
	{
		SpecHandle.Data->SetSetByCallerMagnitude(PassiveAmountTag, Payload->EventMagnitude);

		/*
		 float ConfirmValue = SpecHandle.Data->GetSetByCallerMagnitude(PassiveAmountTag, false, -999);
		UE_LOG(LogTemp, Warning, TEXT("SetByCaller %s Final Value = %f"),
			*PassiveAmountTag.ToString(), ConfirmValue); 
		*/

		/*
		UE_LOG(LogTemp, Warning, TEXT("Applying GE to -> %s"), *ASC->GetOwner()->GetName());
		*/
		
		ASC->ApplyGameplayEffectSpecToSelf(*SpecHandle.Data.Get());
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("SpecHandle INVALID!"));
	}
}
