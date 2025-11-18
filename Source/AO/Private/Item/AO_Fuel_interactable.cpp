// Fill out your copyright notice in the Description page of Project Settings.


#include "Item/AO_Fuel_interactable.h"
#include "AbilitySystemComponent.h"
#include "EngineUtils.h"


// Sets default values
AAO_Fuel_interactable::AAO_Fuel_interactable()
{
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;
	bReplicates = true;
}

// Called when the game starts or when spawned
void AAO_Fuel_interactable::BeginPlay()
{
	Super::BeginPlay();
	
}

void AAO_Fuel_interactable::OnInteractionSuccess_BP_Implementation(AActor* Interactor)
{
	Super::OnInteractionSuccess_BP_Implementation(Interactor);

	if (!HasAuthority()) return;
	if (bConsumed) return;
	bConsumed = true;

	AAO_Train* Train = TargetTrain;
	if (!Train)
	{
		for (TActorIterator<AAO_Train> It(GetWorld()); It; ++It)
		{
			Train = *It;
			break;
		}
	}
	if (!Train) return;

	UAbilitySystemComponent* ASC = Train->GetAbilitySystemComponent();
	if (!ASC)
	{
		UE_LOG(LogTemp, Error, TEXT("❌ Train ASC is null"));
		return;
	}
    
	FGameplayAbilitySpec* FoundSpec = nullptr;
	if (Train->AddEnergyAbilityClass)
	{
		FoundSpec = ASC->FindAbilitySpecFromClass(Train->AddEnergyAbilityClass);
	}
 
	if (!FoundSpec)
	{
		for (FGameplayAbilitySpec& Spec : ASC->GetActivatableAbilities())
		{
			if (Spec.Ability && Spec.Ability->GetClass()->IsChildOf(UAO_AddFuel_GameplayAbility::StaticClass()))
			{
				FoundSpec = &Spec;
				break;
			}
		}
	}

	if (!FoundSpec)
	{
		UE_LOG(LogTemp, Error, TEXT("❌ Train doesn't have AddEnergy ability (after thorough search)"));
		return;
	}
    
	UAO_AddFuel_GameplayAbility* AbilityInst = nullptr;
	if (FoundSpec->GetPrimaryInstance()) // 안전하게 검사
	{
		AbilityInst = Cast<UAO_AddFuel_GameplayAbility>(FoundSpec->GetPrimaryInstance());
	}
    
	if (AbilityInst)
	{
		AbilityInst->PendingAmount = AddFuelAmount;
		UE_LOG(LogTemp, Warning, TEXT("✅ Item: PendingAmount set to ability instance = %f"), AddFuelAmount);
	}
  
	if (FoundSpec)
	{
		ASC->TryActivateAbility(FoundSpec->Handle);
	}

	Destroy();
}
