// AO_ItemCarryComponent.cpp

#include "AI/Component/AO_ItemCarryComponent.h"
#include "AI/Subsystem/AO_AISubsystem.h"
#include "AI/Item/AO_PickupComponent.h"
#include "AI/Controller/AO_CrabController.h"
#include "Item/AO_MasterItem.h"
#include "Net/UnrealNetwork.h"
#include "GameFramework/Pawn.h"
#include "AO_Log.h"

UAO_ItemCarryComponent::UAO_ItemCarryComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
	SetIsReplicatedByDefault(true);
}

void UAO_ItemCarryComponent::BeginPlay()
{
	Super::BeginPlay();
}

void UAO_ItemCarryComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(UAO_ItemCarryComponent, CarriedItem);
}

bool UAO_ItemCarryComponent::TryPickupItem(AAO_MasterItem* Item)
{
	// Item과 Owner가 유효해야 픽업 시도 가능
	if (!ensureMsgf(Item, TEXT("TryPickupItem called with null Item")))
	{
		return false;
	}

	AActor* Owner = GetOwner();
	if (!ensure(Owner))
	{
		return false;
	}

	// 서버에서만 픽업 처리
	if (!Owner->HasAuthority())
	{
		return false;
	}

	if (IsCarryingItem())
	{
		AO_LOG(LogKSJ, Warning, TEXT("Already carrying an item"));
		return false;
	}

	if (!CanPickupItem(Item))
	{
		AO_LOG(LogKSJ, Warning, TEXT("Cannot pickup item: %s"), *Item->GetName());
		return false;
	}

	// AISubsystem에 운반 시작 등록
	if (UAO_AISubsystem* Subsystem = GetAISubsystem())
	{
		if (!Subsystem->TryReserveItem(Item, GetOwner()))
		{
			AO_LOG(LogKSJ, Warning, TEXT("Item already reserved by another AI"));
			return false;
		}
	}

	// 아이템의 PickupComponent를 통해 줍기 - 물리 시뮬레이션 제어 및 부착 처리
	if (UAO_PickupComponent* PickupComp = Item->FindComponentByClass<UAO_PickupComponent>())
	{
		// Owner의 SkeletalMesh에 소켓으로 부착
		USkeletalMeshComponent* OwnerMesh = Owner->FindComponentByClass<USkeletalMeshComponent>();
		if (OwnerMesh && PickupComp->TryPickup(OwnerMesh, PickupSocketName))
		{
			CarriedItem = Item;
			OnItemPickedUp.Broadcast(Item);

			AO_LOG(LogKSJ, Log, TEXT("Item Picked Up: %s"), *Item->GetName());
			return true;
		}
	}

	// 실패 시 예약 해제
	if (UAO_AISubsystem* Subsystem = GetAISubsystem())
	{
		Subsystem->ReleaseItem(Item);
	}

	return false;
}

void UAO_ItemCarryComponent::DropItem()
{
	// 아이템을 들고 있지 않으면 드롭 불가
	if (!CarriedItem)
	{
		return;
	}

	AActor* Owner = GetOwner();
	if (!ensure(Owner))
	{
		return;
	}

	// 서버에서만 드롭 처리
	if (!Owner->HasAuthority())
	{
		return;
	}

	// 드롭할 아이템 캐시 (CarriedItem을 nullptr로 설정하기 전에)
	AAO_MasterItem* DroppedItem = CarriedItem;

	// 아이템의 PickupComponent를 통해 드롭
	if (UAO_PickupComponent* PickupComp = CarriedItem->FindComponentByClass<UAO_PickupComponent>())
	{
		PickupComp->TryDrop();
		PickupComp->SetIgnoreCooldown(DropCooldownTime);
	}

	// AISubsystem에서 예약 해제
	if (UAO_AISubsystem* Subsystem = GetAISubsystem())
	{
		Subsystem->ReleaseItem(DroppedItem);
		Subsystem->MarkItemAsRecentlyDropped(DroppedItem, DropCooldownTime);
	}

	CarriedItem = nullptr;
	OnItemDropped.Broadcast(DroppedItem);

	AO_LOG(LogKSJ, Log, TEXT("Item Dropped: %s"), *DroppedItem->GetName());
}

void UAO_ItemCarryComponent::ForceDropItem()
{
	// 기절 등으로 강제 드롭 시에도 동일하게 처리
	DropItem();
}

AAO_MasterItem* UAO_ItemCarryComponent::FindNearbyItem(float SearchRadius) const
{
	AActor* Owner = GetOwner();
	if (!ensure(Owner))
	{
		return nullptr;
	}

	const FVector OwnerLocation = Owner->GetActorLocation();
	AAO_MasterItem* NearestItem = nullptr;
	float NearestDistSq = SearchRadius * SearchRadius;

	// CrabController에서 발견한 아이템 목록 가져오기 (시야 내 감지된 아이템만 검색)
	AAO_CrabController* CrabController = Cast<AAO_CrabController>(GetAIController());
	if (!CrabController)
	{
		return nullptr;
	}

	// 발견한 아이템 중에서만 검색
	TArray<AAO_MasterItem*> DiscoveredItems = CrabController->GetDiscoveredItems();
	
	for (AAO_MasterItem* Item : DiscoveredItems)
	{
		if (!Item || !CanPickupItem(Item))
		{
			continue;
		}

		const float DistSq = FVector::DistSquared(OwnerLocation, Item->GetActorLocation());
		if (DistSq < NearestDistSq)
		{
			NearestDistSq = DistSq;
			NearestItem = Item;
		}
	}

	return NearestItem;
}

bool UAO_ItemCarryComponent::CanPickupItem(AAO_MasterItem* Item) const
{
	// Item이 유효해야 픽업 가능 여부 확인 가능
	if (!ensureMsgf(Item, TEXT("CanPickupItem called with null Item")))
	{
		return false;
	}

	// 이미 누군가 들고 있는지 확인 (PickupComponent의 상태 체크)
	if (UAO_PickupComponent* PickupComp = Item->FindComponentByClass<UAO_PickupComponent>())
	{
		if (PickupComp->IsPickedUp())
		{
			return false;
		}

		// 쿨다운 중인지 확인
		if (PickupComp->IsInCooldown())
		{
			return false;
		}
	}

	// AISubsystem에서 예약 상태 확인
	if (UAO_AISubsystem* Subsystem = GetAISubsystem())
	{
		if (Subsystem->IsItemReserved(Item))
		{
			return false;
		}

		if (Subsystem->IsItemRecentlyDropped(Item))
		{
			return false;
		}
	}

	return true;
}

UAO_AISubsystem* UAO_ItemCarryComponent::GetAISubsystem() const
{
	if (UWorld* World = GetWorld())
	{
		return World->GetSubsystem<UAO_AISubsystem>();
	}
	return nullptr;
}

AAO_AIControllerBase* UAO_ItemCarryComponent::GetAIController() const
{
	AActor* Owner = GetOwner();
	if (!Owner)
	{
		return nullptr;
	}

	APawn* OwnerPawn = Cast<APawn>(Owner);
	if (!OwnerPawn)
	{
		return nullptr;
	}

	return Cast<AAO_CrabController>(OwnerPawn->GetController());
}

void UAO_ItemCarryComponent::OnRep_CarriedItem()
{
	// 클라이언트에서 아이템 상태 동기화 시 필요한 처리
}
