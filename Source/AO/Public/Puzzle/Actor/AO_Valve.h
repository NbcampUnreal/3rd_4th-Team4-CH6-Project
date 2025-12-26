// HSJ : AO_Valve.h
#pragma once

#include "CoreMinimal.h"
#include "Interaction/Base/AO_BaseInteractable.h"
#include "AO_Valve.generated.h"

class UNiagaraSystem;
class UParticleSystem;

USTRUCT(BlueprintType)
struct FAO_ValveSpawnInfo
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Spawn")
    TSubclassOf<AActor> DamageZoneClass;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Spawn")
    FVector DamageZoneOffset = FVector(100.0f, 0.0f, 0.0f);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Spawn")
    FRotator DamageZoneRotation = FRotator::ZeroRotator;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Spawn|VFX")
    TObjectPtr<UNiagaraSystem> NiagaraEffect;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Spawn|VFX")
    TObjectPtr<UParticleSystem> CascadeEffect;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Spawn|VFX")
    FVector VFXOffset = FVector(100.0f, 0.0f, 0.0f);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Spawn|VFX")
    FRotator VFXRotation = FRotator::ZeroRotator;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Spawn|VFX")
    FVector VFXScale = FVector(1.0f, 1.0f, 1.0f);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Spawn|Sound")
    TObjectPtr<USoundBase> LoopingSound;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Spawn|Sound")
    float SoundVolumeMultiplier = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Spawn|Sound")
    float SoundPitchMultiplier = 1.0f;
};

USTRUCT(BlueprintType)
struct FAO_ValveSpawnedActors
{
    GENERATED_BODY()

    UPROPERTY()
    TObjectPtr<AActor> DamageZone = nullptr;

    UPROPERTY()
    TObjectPtr<UNiagaraComponent> NiagaraComponent = nullptr;

    UPROPERTY()
    TObjectPtr<UParticleSystemComponent> CascadeComponent = nullptr;

    UPROPERTY()
    TObjectPtr<UAudioComponent> AudioComponent = nullptr;
};

UCLASS()
class AO_API AAO_Valve : public AAO_BaseInteractable
{
    GENERATED_BODY()

public:
    AAO_Valve(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

protected:
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
    virtual void OnInteractionSuccess_BP_Implementation(AActor* Interactor) override;
    virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

    UFUNCTION()
    void OnRep_IsValveOpen();

private:
    void OpenValve();
    void CloseValve();
    void SpawnEffects(const FAO_ValveSpawnInfo& SpawnInfo, FAO_ValveSpawnedActors& OutSpawned);
    void CleanupSpawnedActors(FAO_ValveSpawnedActors& SpawnedActors);

public:
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Valve")
    TArray<FAO_ValveSpawnInfo> SpawnInfoArray;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Valve|Sound")
    TObjectPtr<USoundBase> ValveOpenSound;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Valve|Sound")
    TObjectPtr<USoundBase> ValveCloseSound;

protected:
    UPROPERTY(ReplicatedUsing=OnRep_IsValveOpen, BlueprintReadOnly, Category="Valve")
    bool bIsValveOpen = false;

private:
    TArray<FAO_ValveSpawnedActors> SpawnedActorsArray;
};