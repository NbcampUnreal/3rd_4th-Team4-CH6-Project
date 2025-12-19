// HSJ : AO_CannonProjectile.cpp
#include "Puzzle/Actor/Cannon/AO_CannonProjectile.h"
#include "Components/SphereComponent.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "NiagaraFunctionLibrary.h"
#include "Kismet/GameplayStatics.h"
#include "Net/UnrealNetwork.h"
#include "AbilitySystemInterface.h"
#include "AbilitySystemComponent.h"
#include "AO_Log.h"
#include "AI/Character/AO_EnemyBase.h"
#include "Puzzle/Actor/Cannon/AO_CannonProjectilePool.h"

AAO_CannonProjectile::AAO_CannonProjectile()
{
    bReplicates = true;
	SetReplicatingMovement(true);

    CollisionSphere = CreateDefaultSubobject<USphereComponent>(TEXT("CollisionSphere"));
    RootComponent = CollisionSphere;
    CollisionSphere->InitSphereRadius(25.0f);
    CollisionSphere->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
    CollisionSphere->SetCollisionResponseToAllChannels(ECR_Ignore);
    CollisionSphere->SetCollisionResponseToChannel(ECC_WorldStatic, ECR_Block);
    CollisionSphere->SetCollisionResponseToChannel(ECC_WorldDynamic, ECR_Block);
    CollisionSphere->SetCollisionResponseToChannel(ECC_Pawn, ECR_Block);

    ProjectileMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("ProjectileMesh"));
    ProjectileMesh->SetupAttachment(CollisionSphere);
    ProjectileMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);

    ProjectileMovement = CreateDefaultSubobject<UProjectileMovementComponent>(TEXT("ProjectileMovement"));
    ProjectileMovement->InitialSpeed = 2000.0f;
    ProjectileMovement->MaxSpeed = 2000.0f;
    ProjectileMovement->bRotationFollowsVelocity = true;
    ProjectileMovement->ProjectileGravityScale = 0.5f;

    StunEventTag = FGameplayTag::RequestGameplayTag(FName("Event.AI.Stunned"));
}

void AAO_CannonProjectile::BeginPlay()
{
    Super::BeginPlay();

    if (HasAuthority())
    {
        CollisionSphere->OnComponentHit.AddDynamic(this, &AAO_CannonProjectile::OnProjectileHit);
    }
}

void AAO_CannonProjectile::Launch(const FVector& Direction, float Speed)
{
    if (!ProjectileMovement)
    {
    	return;
    }

    ProjectileMovement->Velocity = Direction * Speed;
}

void AAO_CannonProjectile::Activate(const FVector& SpawnLocation, const FRotator& SpawnRotation)
{
    SetActorLocationAndRotation(SpawnLocation, SpawnRotation);
    SetActorHiddenInGame(false);
    SetActorEnableCollision(true);

    if (ProjectileMovement)
    {
        ProjectileMovement->Activate();
    }

    bIsActive = true;
}

void AAO_CannonProjectile::Deactivate()
{
    SetActorHiddenInGame(true);
    SetActorEnableCollision(false);
    
    if (ProjectileMovement)
    {
        ProjectileMovement->Deactivate();
        ProjectileMovement->Velocity = FVector::ZeroVector;
    }

    bIsActive = false;
}

void AAO_CannonProjectile::OnProjectileHit(UPrimitiveComponent* HitComponent, AActor* OtherActor,
    UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit)
{
    if (!HasAuthority() || !bIsActive)
    {
    	return;
    }

    Explode(Hit.ImpactPoint);
}

void AAO_CannonProjectile::Explode(const FVector& Location)
{
    if (!HasAuthority())
    {
    	return;
    }

    // 광역 스턴 처리
    TArray<FHitResult> HitResults;
    FCollisionQueryParams QueryParams;
    QueryParams.AddIgnoredActor(this);

    TObjectPtr<UWorld> World = GetWorld();
    if (!World)
    {
    	return;
    }

    bool bHit = World->SweepMultiByChannel(
        HitResults,
        Location,
        Location,
        FQuat::Identity,
        ECC_Pawn,
        FCollisionShape::MakeSphere(ExplosionRadius),
        QueryParams
    );

    for (const FHitResult& HitResult : HitResults)
    {
        TObjectPtr<AActor> HitActor = HitResult.GetActor();
        if (!HitActor)
        {
            continue;
        }

        // Enemy에게만 Stun 게임플레이 이벤트
    	TObjectPtr<AAO_EnemyBase> Enemy = Cast<AAO_EnemyBase>(HitActor);
        if (!Enemy)
        {
            continue;
        }
    	
    	if (IAbilitySystemInterface* ASI = Cast<IAbilitySystemInterface>(Enemy))
    	{
    		if (TObjectPtr<UAbilitySystemComponent> ASC = ASI->GetAbilitySystemComponent())
    		{
    			FGameplayEventData EventData;
    			EventData.Instigator = GetInstigator();
    			EventData.Target = Enemy;

    			ASC->HandleGameplayEvent(StunEventTag, &EventData);
    		}
    	}
    }

    MulticastExplode(Location);
    ReturnToPool();
}

void AAO_CannonProjectile::MulticastExplode_Implementation(FVector Location)
{
    if (ExplosionVFX)
    {
        UNiagaraFunctionLibrary::SpawnSystemAtLocation(
            this, ExplosionVFX, Location, FRotator::ZeroRotator
        );
    }
    else if (ExplosionCascade)
    {
    	UGameplayStatics::SpawnEmitterAtLocation(
			this, ExplosionCascade, Location, FRotator::ZeroRotator
		);
    }
    if (ExplosionSFX)
    {
        UGameplayStatics::PlaySoundAtLocation(
            this, ExplosionSFX, Location
        );
    }
}

void AAO_CannonProjectile::ReturnToPool()
{
    if (OwnerPool)
    {
        OwnerPool->ReturnProjectile(this);
    }
}