// AO_TrollWeapon.cpp

#include "AI/Item/AO_TrollWeapon.h"
#include "AI/Character/AO_Troll.h"
#include "Components/StaticMeshComponent.h"
#include "Components/SphereComponent.h"
#include "Net/UnrealNetwork.h"
#include "AO_Log.h"

AAO_TrollWeapon::AAO_TrollWeapon()
{
	PrimaryActorTick.bCanEverTick = false;
	bReplicates = true;

	// [Fix] WeaponMesh를 RootComponent로 설정하여 물리 시뮬레이션 시 Actor의 실제 위치가 갱신되도록 변경
	// 기존에는 DetectionSphere가 Root여서, WeaponMesh만 떨어지고 Actor 위치(Root)는 공중에 남는 문제가 있었음.
	WeaponMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("WeaponMesh"));
	RootComponent = WeaponMesh;
	WeaponMesh->SetCollisionProfileName(TEXT("BlockAllDynamic"));
	WeaponMesh->SetSimulatePhysics(false);

	// DetectionSphere를 WeaponMesh의 자식으로 부착
	DetectionSphere = CreateDefaultSubobject<USphereComponent>(TEXT("DetectionSphere"));
	DetectionSphere->SetupAttachment(RootComponent);
	DetectionSphere->SetSphereRadius(100.f);
	DetectionSphere->SetCollisionProfileName(TEXT("OverlapAllDynamic"));
	DetectionSphere->SetGenerateOverlapEvents(true);
}

void AAO_TrollWeapon::BeginPlay()
{
	Super::BeginPlay();

	// 초기 상태: 월드에 놓여있는 상태
	SetCollisionForDropped();
}

void AAO_TrollWeapon::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(AAO_TrollWeapon, OwningTroll);
}

bool AAO_TrollWeapon::PickupByTroll(AAO_Troll* Troll)
{
	if (!Troll)
	{
		AO_LOG(LogKSJ, Warning, TEXT("PickupByTroll: Troll is null"));
		return false;
	}

	// 이미 다른 Troll이 소지 중인 경우
	if (OwningTroll.IsValid() && OwningTroll.Get() != Troll)
	{
		AO_LOG(LogKSJ, Warning, TEXT("PickupByTroll: Weapon already owned by %s"), *OwningTroll->GetName());
		return false;
	}

	OwningTroll = Troll;

	// 물리 시뮬레이션 비활성화
	SetPhysicsEnabled(false);

	// 충돌 설정 변경
	SetCollisionForPickedUp();

	// Troll의 소켓에 부착
	USkeletalMeshComponent* TrollMesh = Troll->GetMesh();
	if (TrollMesh)
	{
		AttachToComponent(TrollMesh, FAttachmentTransformRules::SnapToTargetNotIncludingScale, WeaponSocketName);
	}

	AO_LOG(LogKSJ, Log, TEXT("Weapon picked up by %s"), *Troll->GetName());
	return true;
}

void AAO_TrollWeapon::Drop()
{
	if (!OwningTroll.IsValid())
	{
		return;
	}

	AAO_Troll* PreviousOwner = OwningTroll.Get();
	OwningTroll.Reset();

	// 부착 해제
	DetachFromActor(FDetachmentTransformRules::KeepWorldTransform);

	// 충돌 설정 변경
	SetCollisionForDropped();

	// 물리 시뮬레이션 활성화 (약간의 낙하)
	SetPhysicsEnabled(true);

	AO_LOG(LogKSJ, Log, TEXT("Weapon dropped by %s"), PreviousOwner ? *PreviousOwner->GetName() : TEXT("Unknown"));
}

void AAO_TrollWeapon::OnRep_OwningTroll()
{
	if (OwningTroll.IsValid())
	{
		SetPhysicsEnabled(false);
		SetCollisionForPickedUp();

		USkeletalMeshComponent* TrollMesh = OwningTroll->GetMesh();
		if (TrollMesh)
		{
			AttachToComponent(TrollMesh, FAttachmentTransformRules::SnapToTargetNotIncludingScale, WeaponSocketName);
		}
	}
	else
	{
		DetachFromActor(FDetachmentTransformRules::KeepWorldTransform);
		SetCollisionForDropped();
		SetPhysicsEnabled(true);
	}
}

void AAO_TrollWeapon::SetPhysicsEnabled(bool bEnabled)
{
	if (WeaponMesh)
	{
		WeaponMesh->SetSimulatePhysics(bEnabled);
		
		if (bEnabled)
		{
			WeaponMesh->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
		}
	}
}

void AAO_TrollWeapon::SetCollisionForPickedUp()
{
	// 집어든 상태: 충돌 비활성화
	if (WeaponMesh)
	{
		WeaponMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	}
	if (DetectionSphere)
	{
		DetectionSphere->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	}
}

void AAO_TrollWeapon::SetCollisionForDropped()
{
	// 드롭된 상태: 충돌 활성화
	if (WeaponMesh)
	{
		WeaponMesh->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
		WeaponMesh->SetCollisionProfileName(TEXT("BlockAllDynamic"));
	}
	if (DetectionSphere)
	{
		DetectionSphere->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	}
}
