// HSJ : AO_GasRoomPuzzle.h
#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "GameplayTagContainer.h"
#include "AO_GasRoomPuzzle.generated.h"

class UBoxComponent;
class AAO_BunkerDoor;
class AAO_PasswordPanelInspectable;
class AAO_PuzzleConditionChecker;
class UNiagaraSystem;
class UNiagaraComponent;
class UParticleSystem;
class UParticleSystemComponent;
class UAudioComponent;
class UDecalComponent;

UENUM(BlueprintType)
enum class EGasRoomState : uint8
{
    Idle,
    WaitingToStart,
    Active,
    Completed
};

USTRUCT(BlueprintType)
struct FAO_GasEffectSpawnInfo
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Effect")
    FVector RelativeLocation = FVector::ZeroVector;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Effect")
    FRotator RelativeRotation = FRotator::ZeroRotator;

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
};

USTRUCT(BlueprintType)
struct FAO_GasEffectSpawnedActors
{
    GENERATED_BODY()

    UPROPERTY()
    UNiagaraComponent* NiagaraComponent = nullptr;

    UPROPERTY()
    UParticleSystemComponent* CascadeComponent = nullptr;

    UPROPERTY()
    UAudioComponent* AudioComponent = nullptr;
};

UCLASS()
class AO_API AAO_GasRoomPuzzle : public AActor
{
    GENERATED_BODY()

public:
    AAO_GasRoomPuzzle();

    virtual void PostInitializeComponents() override;

protected:
    virtual void BeginPlay() override;
    virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
    virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

    UFUNCTION()
    void OnBoxBeginOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, 
        UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

    UFUNCTION()
    void OnBoxEndOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, 
        UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);

    UFUNCTION(NetMulticast, Reliable)
    void MulticastSpawnGasEffects();

    UFUNCTION(NetMulticast, Reliable)
    void MulticastCleanupGasEffects();

    UFUNCTION(NetMulticast, Reliable)
    void MulticastOpenDoors();

    UFUNCTION(NetMulticast, Reliable)
    void MulticastCloseDoors();

    UFUNCTION(NetMulticast, Reliable)
	void MulticastUpdatePasswordHints(int32 Digit1, int32 Digit2, int32 Digit3, int32 Digit4,
		int32 Idx1, int32 Idx2, int32 Idx3, int32 Idx4); 

    UFUNCTION(NetMulticast, Reliable)
    void MulticastClearPasswordHints();

private:
    void CollectCandidateDecals();
    void SelectRandomCandidates();
    
    void StartPuzzleDelayed();
    void StartPuzzle();
    void CheckAliveCharacters();
    void OnCountdownTick();
    void CompletePuzzle();
    void ResetPuzzle();
    
    void ActivateDamageZone();
    void DeactivateDamageZone();
    
    int32 GetAliveCharacterCount() const;
    void ClearAllTimers();

    UFUNCTION()
    void OnPasswordCorrect();

    void LockDoors();
    void UnlockDoors();

public:
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Components")
    TObjectPtr<UBoxComponent> TriggerBox;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Components")
    TObjectPtr<USceneComponent> RootComp;

    UPROPERTY(EditInstanceOnly, BlueprintReadWrite, Category="GasRoom|Setup")
    TArray<TObjectPtr<AAO_BunkerDoor>> BunkerDoors;

    UPROPERTY(EditInstanceOnly, BlueprintReadWrite, Category="GasRoom|Setup")
    TObjectPtr<AAO_PasswordPanelInspectable> PasswordPanel;

    UPROPERTY(EditInstanceOnly, BlueprintReadWrite, Category="GasRoom|Setup")
    TObjectPtr<AAO_PuzzleConditionChecker> LinkedChecker;

    UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category="GasRoom|Password Hints")
    TArray<TObjectPtr<UDecalComponent>> CandidateDecals;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="GasRoom|Password Hints")
    FString CandidateDecalPrefix = TEXT("Candidate_");

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="GasRoom|Password Hints")
    FLinearColor Digit1Color = FLinearColor::Red;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="GasRoom|Password Hints")
    FLinearColor Digit2Color = FLinearColor::Yellow;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="GasRoom|Password Hints")
    FLinearColor Digit3Color = FLinearColor::Green;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="GasRoom|Password Hints")
    FLinearColor Digit4Color = FLinearColor::Blue;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="GasRoom|Password Hints")
    TArray<TObjectPtr<UTexture2D>> NumberTextures;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="GasRoom|Password Hints")
    TObjectPtr<UMaterialInterface> NumberDecalMaterial;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="GasRoom|Settings")
    FGameplayTag CompletionTag;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="GasRoom|Settings")
    FGameplayTag DeathTag;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="GasRoom|Timing")
    float DelayBeforeStart = 3.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="GasRoom|Timing")
    float CountdownDuration = 60.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="GasRoom|Timing")
    float CharacterCheckInterval = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="GasRoom|Timing")
    float CountdownTickInterval = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="GasRoom|Effects")
    TArray<FAO_GasEffectSpawnInfo> GasEffectSpawnInfos;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="GasRoom|Damage")
    TSubclassOf<AActor> DamageZoneClass;

protected:
    UPROPERTY(Replicated)
    EGasRoomState CurrentState = EGasRoomState::Idle;

    UPROPERTY(Replicated)
    float RemainingTime = 0.0f;

    UPROPERTY(Replicated)
    int32 ActiveDecalIndices[4];

private:
    TSet<TWeakObjectPtr<AActor>> OverlappedCharacters;
    
    FTimerHandle StartDelayTimer;
    FTimerHandle CharacterCheckTimer;
    FTimerHandle CountdownTimer;

    TArray<FAO_GasEffectSpawnedActors> SpawnedEffects;

    UPROPERTY()
    TObjectPtr<AActor> SpawnedDamageZone;

    bool bHasEverStarted = false;
};