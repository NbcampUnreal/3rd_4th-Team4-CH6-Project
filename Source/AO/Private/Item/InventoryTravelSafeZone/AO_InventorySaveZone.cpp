#include "Item/InventoryTravelSafeZone/AO_InventorySaveZone.h"
#include "Components/BoxComponent.h"
#include "Character/AO_PlayerCharacter.h"
#include "Player/PlayerState/AO_PlayerState.h"

AAO_InventorySaveZone::AAO_InventorySaveZone()
{
	PrimaryActorTick.bCanEverTick = false;

	Zone = CreateDefaultSubobject<UBoxComponent>(TEXT("Zone"));
	SetRootComponent(Zone);

	Zone->ComponentTags.Add("InventorySafeZone");
	Zone->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	Zone->SetCollisionResponseToAllChannels(ECR_Ignore);
	Zone->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);
	Zone->SetGenerateOverlapEvents(true);
}

void AAO_InventorySaveZone::BeginPlay()
{
	Super::BeginPlay();

	Zone->OnComponentBeginOverlap.AddDynamic(this, &AAO_InventorySaveZone::OnEnterSafeZone);
	Zone->OnComponentEndOverlap.AddDynamic(this, &AAO_InventorySaveZone::OnExitSafeZone);

	if (HasAuthority())
	{
		TArray<AActor*> InsideActors;
		Zone->GetOverlappingActors(InsideActors, AAO_PlayerCharacter::StaticClass());

		for (AActor* Actor : InsideActors)
		{
			if (AAO_PlayerCharacter* PC = Cast<AAO_PlayerCharacter>(Actor))
			{
				if (AAO_PlayerState* PS = PC->GetPlayerState<AAO_PlayerState>())
				{
					PS->SetSafeZoneState(true);
				}
			}
		}
	}
}

void AAO_InventorySaveZone::OnEnterSafeZone(
	UPrimitiveComponent* OverlappedComp,
	AActor* OtherActor,
	UPrimitiveComponent* OtherComp,
	int32 BodyIndex,
	bool bFromSweep,
	const FHitResult& Hit)
{
	if (!HasAuthority())
		return;

	if (AAO_PlayerCharacter* PC = Cast<AAO_PlayerCharacter>(OtherActor))
	{
		if (AAO_PlayerState* PS = PC->GetPlayerState<AAO_PlayerState>())
		{
			PS->SetSafeZoneState(true);
		}
	}
}

void AAO_InventorySaveZone::OnExitSafeZone(
	UPrimitiveComponent* OverlappedComp,
	AActor* OtherActor,
	UPrimitiveComponent* OtherComp,
	int32 BodyIndex)
{
	if (!HasAuthority())
		return;

	if (AAO_PlayerCharacter* PC = Cast<AAO_PlayerCharacter>(OtherActor))
	{
		if (AAO_PlayerState* PS = PC->GetPlayerState<AAO_PlayerState>())
		{
			UE_LOG(LogTemp, Log, TEXT("[Zone] Exit 호출됨: Player(%s), bIsTraveling(%s)"), 
				*PS->GetPlayerName(), PS->bIsTraveling ? TEXT("True") : TEXT("False"));
            
			if (PS->bIsTraveling) return;
			PS->SetSafeZoneState(false);
		}
	}
}