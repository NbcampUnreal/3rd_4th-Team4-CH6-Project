// HSJ : AO_RollingBall.h
#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "GameplayTagContainer.h"
#include "AO_RollingBall.generated.h"

class UGameplayEffect;
class AAO_DestructibleCacheActor;

UCLASS()
class AO_API AAO_RollingBall : public AActor
{
    GENERATED_BODY()

public:
    AAO_RollingBall();

    UFUNCTION(BlueprintCallable, Category="RollingBall")
    void ResetToStart();

    UFUNCTION(BlueprintCallable, Category="RollingBall")
    void SetShouldMove(bool bInShouldMove);

    UFUNCTION(BlueprintPure, Category="RollingBall")
    bool GetShouldMove() const { return bShouldMove; }

    UFUNCTION(BlueprintPure, Category="RollingBall")
    UStaticMeshComponent* GetBallMesh() const { return BallMesh; }

protected:
    virtual void BeginPlay() override;
    virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

    UFUNCTION()
    void OnBallBeginOverlap(
        UPrimitiveComponent* OverlappedComponent,
        AActor* OtherActor,
        UPrimitiveComponent* OtherComp,
        int32 OtherBodyIndex,
        bool bFromSweep,
        const FHitResult& SweepResult
    );

public:
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Components")
    TObjectPtr<UStaticMeshComponent> BallMesh;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="RollingBall|Damage")
    float Damage = -30.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="RollingBall|Knockback")
    float KnockbackStrength = 1500.0f;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="RollingBall|Effects")
    TSubclassOf<UGameplayEffect> DamageEffectClass;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="RollingBall|Linked")
	TObjectPtr<AAO_DestructibleCacheActor> LinkedDestructible;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="RollingBall|Effects")
    FGameplayTag KnockdownHitReactTag;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="RollingBall|Cooldown")
    float HitCooldown = 1.0f;

protected:
    UPROPERTY(Replicated, BlueprintReadOnly)
    FVector StartPosition;

    UPROPERTY(Replicated, BlueprintReadOnly)
    bool bShouldMove = false;

    FGameplayTag DeathTag;

private:
    TMap<TWeakObjectPtr<AActor>, float> PlayerHitCooldowns;
};