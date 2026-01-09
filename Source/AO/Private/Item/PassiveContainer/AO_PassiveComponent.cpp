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
    if (!Payload || !GetOwner()) return;

    UAbilitySystemComponent* ASC = GetOwner()->FindComponentByClass<UAbilitySystemComponent>();
    if (!ASC) return;

    TSubclassOf<UGameplayEffect> SelectedPassive = nullptr;
    bool bIsMaxHPEvent = false;
	
    FGameplayTag MaxHPTag = FGameplayTag::RequestGameplayTag(TEXT("Event.Interaction.AddPassive.MaxHP"));
    
    if (Payload->EventTag.MatchesTag(MaxHPTag))
    {
        SelectedPassive = MaxHpPassive;
        bIsMaxHPEvent = true;
    }
    else if (Payload->EventTag.MatchesTag(FGameplayTag::RequestGameplayTag(TEXT("Event.Interaction.AddPassive.Stamina"))))
    {
        SelectedPassive = MaxStaminaPassive;
    }
    else if (Payload->EventTag.MatchesTag(FGameplayTag::RequestGameplayTag(TEXT("Event.Interaction.AddPassive.MoveSpeed"))))
    {
        SelectedPassive = MovementPassive;
    }

    if (!SelectedPassive) return;
	
    FGameplayEffectContextHandle MaxContext = ASC->MakeEffectContext();
    FGameplayEffectSpecHandle MaxSpecHandle = ASC->MakeOutgoingSpec(SelectedPassive, 1.f, MaxContext);
    if (MaxSpecHandle.IsValid())
    {
        static FGameplayTag PassiveAmountTag = FGameplayTag::RequestGameplayTag(TEXT("Data.PassiveAmount"));
        MaxSpecHandle.Data->SetSetByCallerMagnitude(PassiveAmountTag, Payload->EventMagnitude);
        ASC->ApplyGameplayEffectSpecToSelf(*MaxSpecHandle.Data.Get());
    }
    if (bIsMaxHPEvent && AddHpPassive)
    {
        FGameplayEffectContextHandle HealthContext = ASC->MakeEffectContext();
        FGameplayEffectSpecHandle HealthSpecHandle = ASC->MakeOutgoingSpec(AddHpPassive, 1.f, HealthContext);
        
        if (HealthSpecHandle.IsValid())
        {
            static FGameplayTag RecoveryTag = FGameplayTag::RequestGameplayTag(TEXT("Data.Item")); 
            
            HealthSpecHandle.Data->SetSetByCallerMagnitude(RecoveryTag, Payload->EventMagnitude);
        	
            FActiveGameplayEffectHandle ActiveHandle = ASC->ApplyGameplayEffectSpecToSelf(*HealthSpecHandle.Data.Get());
        }
    }
}
