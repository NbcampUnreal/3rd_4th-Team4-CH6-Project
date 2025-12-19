// HSJ : AO_CannonProjectilePool.cpp
#include "Puzzle/Actor/Cannon/AO_CannonProjectilePool.h"
#include "Puzzle/Actor/Cannon/AO_CannonProjectile.h"
#include "AO_Log.h"

UAO_CannonProjectilePool::UAO_CannonProjectilePool()
{
    PrimaryComponentTick.bCanEverTick = false;
}

void UAO_CannonProjectilePool::BeginPlay()
{
    Super::BeginPlay();

    if (GetOwner()->HasAuthority())
    {
        InitializePool();
    }
}

void UAO_CannonProjectilePool::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
    ClearPool();
    Super::EndPlay(EndPlayReason);
}

void UAO_CannonProjectilePool::InitializePool()
{
    if (!ProjectileClass)
    {
    	return;
    }

    TObjectPtr<UWorld> World = GetWorld();
    if (!World)
    {
    	return;
    }

    for (int32 i = 0; i < PoolSize; ++i)
    {
        FActorSpawnParameters SpawnParams;
        SpawnParams.Owner = GetOwner();
        SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

        TObjectPtr<AAO_CannonProjectile> Projectile = World->SpawnActor<AAO_CannonProjectile>(
            ProjectileClass,
            FVector::ZeroVector,
            FRotator::ZeroRotator,
            SpawnParams
        );

        if (Projectile)
        {
        	Projectile->SetOwnerPool(this);
            Projectile->Deactivate();
            AvailableProjectiles.Add(Projectile);
        }
    }
}

void UAO_CannonProjectilePool::ClearPool()
{
    for (TObjectPtr<AAO_CannonProjectile> Projectile : AvailableProjectiles)
    {
        if (Projectile && IsValid(Projectile))
        {
            Projectile->Destroy();
        }
    }

    for (TObjectPtr<AAO_CannonProjectile> Projectile : ActiveProjectiles)
    {
        if (Projectile && IsValid(Projectile))
        {
            Projectile->Destroy();
        }
    }

    AvailableProjectiles.Empty();
    ActiveProjectiles.Empty();
}

AAO_CannonProjectile* UAO_CannonProjectilePool::GetProjectile()
{
	// 사용 가능한 투사체가 있으면 반환
	if (AvailableProjectiles.Num() > 0)
	{
		TObjectPtr<AAO_CannonProjectile> Projectile = AvailableProjectiles.Pop();
		ActiveProjectiles.Add(Projectile);
		return Projectile;
	}

	// 풀이 비었으면 가장 오래된 활성 투사체를 강제 회수
	if (ActiveProjectiles.Num() > 0)
	{
		TObjectPtr<AAO_CannonProjectile> OldestProjectile = ActiveProjectiles[0];
        
		// 강제로 풀에 반환 (폭발 이펙트는 재생하지 않음)
		OldestProjectile->Deactivate();
		ActiveProjectiles.RemoveAt(0);
		AvailableProjectiles.Add(OldestProjectile);
        
		// 다시 가져오기
		return GetProjectile();
	}

	return nullptr;
}

void UAO_CannonProjectilePool::ReturnProjectile(AAO_CannonProjectile* Projectile)
{
    if (!Projectile)
    {
    	return;
    }

    Projectile->Deactivate();
    
    ActiveProjectiles.Remove(Projectile);
    AvailableProjectiles.Add(Projectile);
}