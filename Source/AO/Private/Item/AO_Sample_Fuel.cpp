#include "Item/AO_Sample_Fuel.h"
#include "EngineUtils.h"
#include "AbilitySystemComponent.h"
#include "Train/GAS/AO_AddFuel_GameplayAbility.h"
#include "Train/AO_Train.h"

AAO_Sample_Fuel::AAO_Sample_Fuel()
{
	PrimaryActorTick.bCanEverTick = false;
	bReplicates = true;

	FuelMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("FuelMesh"));
	SetRootComponent(FuelMesh);

	FuelMesh->SetCollisionProfileName(TEXT("OverlapAll"));
	FuelMesh->SetGenerateOverlapEvents(true);
	FuelMesh->OnComponentBeginOverlap.AddDynamic(this, &AAO_Sample_Fuel::OnOverlap);
}

void AAO_Sample_Fuel::BeginPlay()
{
	Super::BeginPlay();
}

void AAO_Sample_Fuel::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

void AAO_Sample_Fuel::OnOverlap(UPrimitiveComponent* Overlapped, AActor* OtherActor,
    UPrimitiveComponent* OtherComp, int32 OtherBodyIndex,
    bool bFromSweep, const FHitResult& SweepResult)

{
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
    }
  
    if (FoundSpec)
    {
        ASC->TryActivateAbility(FoundSpec->Handle);
    }

    Destroy();
}