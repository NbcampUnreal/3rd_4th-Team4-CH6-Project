// AO_PickupComponent.cpp

#include "AI/Item/AO_PickupComponent.h"
#include "Net/UnrealNetwork.h"
#include "Components/PrimitiveComponent.h"
#include "Engine/World.h"

UAO_PickupComponent::UAO_PickupComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
	SetIsReplicatedByDefault(true);
}

void UAO_PickupComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(UAO_PickupComponent, bIsPickedUp);
	DOREPLIFETIME(UAO_PickupComponent, ItemTags);
}

bool UAO_PickupComponent::TryPickup(USceneComponent* AttachTo, FName SocketName)
{
	// AttachTo가 유효해야 부착 가능
	if (!ensureMsgf(AttachTo, TEXT("TryPickup called with null AttachTo")))
	{
		return false;
	}

	// 이미 픽업된 상태면 실패
	if (bIsPickedUp)
	{
		return false;
	}
	
	// 쿨다운 중이면 픽업 불가
	if (IsInCooldown())
	{
		return false;
	}

	AActor* Owner = GetOwner();
	if (!ensure(Owner))
	{
		return false;
	}

	// 물리 끄기 - 부착 전 물리 시뮬레이션 비활성화
	ConfigurePhysics(false);

	// 부착 - 지정된 소켓에 아이템 부착
	bool bAttached = Owner->AttachToComponent(AttachTo, FAttachmentTransformRules::SnapToTargetNotIncludingScale, SocketName);
	if (bAttached)
	{
		bIsPickedUp = true;
		CooldownExpireTime = 0.0;
	}
	else
	{
		// 실패 시 물리 복구
		ConfigurePhysics(true);
	}

	return bAttached;
}

void UAO_PickupComponent::TryDrop()
{
	// 픽업 상태가 아니면 드롭 불가
	if (!bIsPickedUp)
	{
		return;
	}

	AActor* Owner = GetOwner();
	if (!ensure(Owner))
	{
		return;
	}

	// 부착 해제 - 현재 월드 위치 유지
	Owner->DetachFromActor(FDetachmentTransformRules::KeepWorldTransform);

	// 물리 켜기 - 드롭 후 물리 시뮬레이션 활성화
	ConfigurePhysics(true);

	bIsPickedUp = false;
	
	// 드롭 후 쿨다운 설정 - 바로 다시 줍지 못하도록
	SetIgnoreCooldown(5.0f);
}

bool UAO_PickupComponent::HasTag(FGameplayTag TagToCheck) const
{
	return ItemTags.HasTag(TagToCheck);
}

void UAO_PickupComponent::AddTag(FGameplayTag TagToAdd)
{
	if (TagToAdd.IsValid())
	{
		ItemTags.AddTag(TagToAdd);
	}
}

void UAO_PickupComponent::OnRep_IsPickedUp()
{
	ConfigurePhysics(!bIsPickedUp);
}

void UAO_PickupComponent::ConfigurePhysics(bool bEnablePhysics)
{
	// 첫 호출 시 RootComponent를 TargetPhysicsComp로 캐싱
	if (!TargetPhysicsComp)
	{
		AActor* Owner = GetOwner();
		if (Owner)
		{
			TargetPhysicsComp = Cast<UPrimitiveComponent>(Owner->GetRootComponent());
		}
	}

	// 물리 컴포넌트가 있으면 물리 시뮬레이션 및 콜리전 설정
	if (TargetPhysicsComp)
	{
		TargetPhysicsComp->SetSimulatePhysics(bEnablePhysics);
		if (bEnablePhysics)
		{
			// 물리 활성화 시 QueryAndPhysics로 충돌 처리
			TargetPhysicsComp->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
		}
		else
		{
			// 물리 비활성화 시 NoCollision으로 충돌 무시 (부착 상태)
			TargetPhysicsComp->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		}
	}
}

void UAO_PickupComponent::SetIgnoreCooldown(float Duration)
{
	UWorld* World = GetWorld();
	if (World)
	{
		CooldownExpireTime = World->GetTimeSeconds() + Duration;
	}
}

bool UAO_PickupComponent::IsInCooldown() const
{
	UWorld* World = GetWorld();
	if (World)
	{
		return World->GetTimeSeconds() < CooldownExpireTime;
	}
	return false;
}