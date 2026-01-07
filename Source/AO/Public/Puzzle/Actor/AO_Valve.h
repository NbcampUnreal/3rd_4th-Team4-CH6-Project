// HSJ : AO_Valve.h
#pragma once

#include "CoreMinimal.h"
#include "Interaction/Base/AO_BaseInteractable.h"
#include "AO_Valve.generated.h"

class UNiagaraSystem;
class UParticleSystem;
class UBoxComponent;

USTRUCT(BlueprintType)
struct FAO_ValveEffectSpawnInfo
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Effect|VFX")
    TObjectPtr<UNiagaraSystem> NiagaraEffect;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Effect|VFX")
    TObjectPtr<UParticleSystem> CascadeEffect;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Effect|VFX")
    FVector VFXScale = FVector(1.0f);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Effect|Sound")
    TObjectPtr<USoundBase> LoopingSound;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Effect|Sound")
    float SoundVolumeMultiplier = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Effect|Sound")
    float SoundPitchMultiplier = 1.0f;
};

USTRUCT(BlueprintType)
struct FAO_ValveSpawnedActors
{
    GENERATED_BODY()

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

    virtual void PostInitializeComponents() override;

protected:
	virtual void BeginPlay() override;
    virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
    virtual void OnInteractionSuccess_BP_Implementation(AActor* Interactor) override;
    virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

    UFUNCTION()
    void OnRep_IsValveOpen();

private:
    void CollectVFXPoints();
    void CollectDamageZoneBoxes();
    void OpenValve();
    void CloseValve();
    void SpawnDamageZones();
    void DestroyDamageZones();
    void SpawnEffects();
    void CleanupEffects();

public:
    UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category="Valve|VFX")
    TArray<TObjectPtr<USceneComponent>> VFXPoints;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Valve|VFX")
    FString VFXPointPrefix = TEXT("VFX_");

    UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category="Valve|Damage")
    TArray<TObjectPtr<UBoxComponent>> DamageZoneBoxes;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Valve|Damage")
    FString DamageZonePrefix = TEXT("DamageZone_");

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Valve|Effects")
    TArray<FAO_ValveEffectSpawnInfo> EffectInfoArray;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Valve|Damage")
    TSubclassOf<AActor> DamageZoneClass;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Valve|Sound")
    TObjectPtr<USoundBase> ValveOpenSound;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Valve|Sound")
    TObjectPtr<USoundBase> ValveCloseSound;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Valve|Interaction")
    float InteractionLockDuration = 2.0f;

protected:
    UPROPERTY(ReplicatedUsing=OnRep_IsValveOpen, EditAnywhere, BlueprintReadOnly, Category="Valve")
    bool bIsValveOpen = false;

private:
    UPROPERTY()
    TArray<TObjectPtr<AActor>> SpawnedDamageZones;

    TArray<FAO_ValveSpawnedActors> SpawnedEffectsArray;

    FTimerHandle InteractionLockTimer;
};