// HSJ : AO_CannonProjectile.h
#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "GameplayTagContainer.h"
#include "AO_CannonProjectile.generated.h"

class USphereComponent;
class UProjectileMovementComponent;
class UNiagaraSystem;
class UAO_CannonProjectilePool;

UCLASS()
class AO_API AAO_CannonProjectile : public AActor
{
    GENERATED_BODY()

public:
    AAO_CannonProjectile();
	
    UFUNCTION(BlueprintCallable, Category = "Cannon")
    void Launch(const FVector& Direction, float Speed);
	
    UFUNCTION(BlueprintCallable, Category = "Cannon")
    void ReturnToPool();

    void Activate(const FVector& SpawnLocation, const FRotator& SpawnRotation);
    void Deactivate();

	void SetOwnerPool(UAO_CannonProjectilePool* Pool) { OwnerPool = Pool; }

protected:
    virtual void BeginPlay() override;

    UFUNCTION()
    void OnProjectileHit(UPrimitiveComponent* HitComponent, AActor* OtherActor, 
        UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit);

    void Explode(const FVector& Location);

    UFUNCTION(NetMulticast, Reliable)
    void MulticastExplode(FVector Location);

public:
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    TObjectPtr<USphereComponent> CollisionSphere;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    TObjectPtr<UStaticMeshComponent> ProjectileMesh;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    TObjectPtr<UProjectileMovementComponent> ProjectileMovement;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cannon")
    float ExplosionRadius = 500.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cannon")
    TObjectPtr<UNiagaraSystem> ExplosionVFX;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cannon")
	TObjectPtr<UParticleSystem> ExplosionCascade;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cannon")
    TObjectPtr<USoundBase> ExplosionSFX;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cannon")
    FGameplayTag StunEventTag;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cannon")
	FGameplayTag DestructionTriggerTag;

private:
    UPROPERTY()
    TObjectPtr<class UAO_CannonProjectilePool> OwnerPool;

    bool bIsActive = false;
};