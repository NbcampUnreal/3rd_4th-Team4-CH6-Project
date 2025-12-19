// HSJ : AO_CannonProjectilePool.h
#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "AO_CannonProjectilePool.generated.h"

class AAO_CannonProjectile;

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class AO_API UAO_CannonProjectilePool : public UActorComponent
{
	GENERATED_BODY()

public:
	UAO_CannonProjectilePool();

	UFUNCTION(BlueprintCallable, Category = "Cannon")
	AAO_CannonProjectile* GetProjectile();

	void ReturnProjectile(AAO_CannonProjectile* Projectile);

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cannon")
	TSubclassOf<AAO_CannonProjectile> ProjectileClass;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cannon")
	int32 PoolSize = 10;

protected:
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

private:
	void InitializePool();
	void ClearPool();

	UPROPERTY()
	TArray<TObjectPtr<AAO_CannonProjectile>> AvailableProjectiles;

	UPROPERTY()
	TArray<TObjectPtr<AAO_CannonProjectile>> ActiveProjectiles;
};