#include "Item/AO_MasterItem.h"
#include "Components/StaticMeshComponent.h"
#include "AbilitySystemComponent.h"
#include "Item/AO_struct_FItemBase.h"
#include "Train/GAS/AO_AddFuel_GameplayAbility.h"

AAO_MasterItem::AAO_MasterItem()
{
	PrimaryActorTick.bCanEverTick = false;
	bReplicates = true;

	// Interaction Sphere
	InteractionSphere = CreateDefaultSubobject<USphereComponent>(TEXT("InteractionSphere"));
	InteractionSphere->SetupAttachment(MeshComponent);
	InteractionSphere->SetSphereRadius(50.f);
	InteractionSphere->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	InteractionSphere->SetCollisionResponseToAllChannels(ECR_Ignore);
	InteractionSphere->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);

	// Components
	PickupComponent = CreateDefaultSubobject<UAO_ItemPickupComponent>(TEXT("PickupComponent"));
	FuelComponent   = CreateDefaultSubobject<UAO_ItemFuelComponent>(TEXT("FuelComponent"));
}

void AAO_MasterItem::BeginPlay()
{
	Super::BeginPlay();

	if (!ItemID.IsNone() && ItemDataTable)
	{
		static const FString ContextString(TEXT("Reading ItemDataTable"));
		if (FAO_struct_FItemBase* Row = ItemDataTable->FindRow<FAO_struct_FItemBase>(ItemID, ContextString))
		{
			ItemTags = Row->ItemTags;

			if (FuelComponent)
				FuelComponent->AddFuelAmount = Row->FuelAmount;

			if (!Row->WorldMesh.IsNull())
			{
				if (UStaticMesh* Mesh = Row->WorldMesh.LoadSynchronous())
					MeshComponent->SetStaticMesh(Mesh);
			}
		}
	}
}

void AAO_MasterItem::OnInteractionSuccess_BP_Implementation(AActor* Interactor)
{
	Super::OnInteractionSuccess_BP_Implementation(Interactor);

	UE_LOG(LogTemp, Warning, TEXT("AAO_MasterItem::OnInteractionSuccess_BP_Implementation called"));

	// Fuel 적용
	if (FuelComponent && Interactor)
	{
		FuelComponent->ApplyFuel();
	}

	// Pickup 처리 (optional)
	if (PickupComponent && Interactor)
	{
		if (USkeletalMeshComponent* SkeletalMesh = Interactor->FindComponentByClass<USkeletalMeshComponent>())
		{
			PickupComponent->Pickup(SkeletalMesh, FName("PickupSocket"));
		}
	}

	// 아이템 Destroy
	if (HasAuthority())
	{
		Destroy();
	}
}

void AAO_MasterItem::GetOwnedGameplayTags(FGameplayTagContainer& TagContainer) const
{
	TagContainer = ItemTags;
}

void AAO_MasterItem::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(AAO_MasterItem, ItemTags);
}
