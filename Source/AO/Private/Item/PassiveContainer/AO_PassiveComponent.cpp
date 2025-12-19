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
	//UE_LOG(LogTemp, Warning, TEXT("OnGameplayEventReceived() Triggered"));

	if (!Payload)
	{
		//UE_LOG(LogTemp, Warning, TEXT("Payload NULL or PassiveEffectClass NOT set"));
		return;
	}

	UAbilitySystemComponent* ASC = GetOwner()->FindComponentByClass<UAbilitySystemComponent>();
	if (!ASC)
	{
		//UE_LOG(LogTemp, Warning, TEXT("ASC NOT Found on Owner"));
		return;
	}

	static FGameplayTag PassiveAmountTag = FGameplayTag::RequestGameplayTag(TEXT("Data.PassiveAmount"));
	FGameplayEffectContextHandle Context = ASC->MakeEffectContext();
	
	TSubclassOf<UGameplayEffect> SelectedPassive = MaxHpPassive;
	UE_LOG(LogTemp, Warning, TEXT("EventTag = %s"), *Payload->EventTag.ToString());
	if (Payload->EventTag.ToString() == "Event.Interaction.AddPassive.Stamina")
	{
		//UE_LOG(LogTemp, Warning, TEXT("EventTag = %s"), *Payload->EventTag.ToString());
		SelectedPassive = MaxStaminaPassive;
	}
	else if (Payload->EventTag.ToString() == "Event.Interaction.AddPassive.MoveSpeed")
	{
		//UE_LOG(LogTemp, Warning, TEXT("EventTag = %s"), *Payload->EventTag.ToString());
		SelectedPassive = MovementPassive;
	}

	FGameplayEffectSpecHandle SpecHandle = ASC->MakeOutgoingSpec(SelectedPassive, 1.f, Context);

	if (!SelectedPassive)
	{
		//UE_LOG(LogTemp, Error, TEXT("SelectedPassive is NULL"));
		return;
	}
	if (SpecHandle.IsValid())
	{
		//UE_LOG(LogTemp, Warning, TEXT("SpecHandle VALID!"));
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
		//UE_LOG(LogTemp, Warning, TEXT("SpecHandle INVALID!"));
	}
	
}
