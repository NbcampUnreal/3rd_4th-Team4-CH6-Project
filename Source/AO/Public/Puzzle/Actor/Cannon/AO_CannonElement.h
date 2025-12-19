// HSJ : AO_CannonElement.h
#pragma once

#include "CoreMinimal.h"
#include "NiagaraSystem.h"
#include "Puzzle/Element/AO_InspectionPuzzle.h"
#include "AO_CannonElement.generated.h"

class UAO_CannonProjectilePool;

UCLASS()
class AO_API AAO_CannonElement : public AAO_InspectionPuzzle
{
    GENERATED_BODY()

public:
    AAO_CannonElement(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

    virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
    virtual FAO_InspectionCameraSettings GetInspectionCameraSettings() const override;
    virtual void OnInspectionInput(const FVector2D& InputValue, float DeltaTime) override;
    virtual void OnInspectionInputLocal(const FVector2D& InputValue, float DeltaTime) override;
    virtual void OnInspectionAction() override;

	void OnInspectionStarted();
	void OnInspectionEnded();

protected:
    virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

    UFUNCTION(NetMulticast, Reliable)
    void MulticastPlayFireEffect();

    UFUNCTION()
    void OnRep_ServerYaw();
    UFUNCTION()
    void OnRep_ServerPitch();
	UFUNCTION()
	void OnRep_CurrentOperator();

private:
    void UpdateBarrelRotation(float Yaw, float Pitch);
    void UpdateCameraTransform();
    void ProcessRotation(const FVector2D& InputValue, float DeltaTime, float& OutYaw, float& OutPitch);

	void StartInterpolation();
	void StopInterpolation();
	void UpdateInterpolation();
	bool IsLocalPlayerOperating() const;

public:
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    TObjectPtr<UStaticMeshComponent> BaseMesh;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    TObjectPtr<UStaticMeshComponent> BarrelMesh;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    TObjectPtr<USceneComponent> MuzzlePoint;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    TObjectPtr<UAO_CannonProjectilePool> ProjectilePool;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cannon|Rotation")
    float RotationSpeed = 30.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cannon|Rotation")
    float MaxYawAngle = 60.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cannon|Rotation")
    float MinPitchAngle = -30.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cannon|Rotation")
    float MaxPitchAngle = 30.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cannon|Fire")
    float ProjectileSpeed = 2000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cannon|Fire")
    float FireCooldown = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cannon|Fire")
    TObjectPtr<UNiagaraSystem> MuzzleFlashVFX;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cannon|Fire")
    TObjectPtr<UParticleSystem> MuzzleFlashCascade;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cannon|Fire")
    TObjectPtr<USoundBase> FireSFX;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cannon|Rotation")
	float RotationInterpSpeed = 15.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cannon|Rotation")
	float InterpUpdateRate = 0.016f;

protected:
	UPROPERTY(ReplicatedUsing=OnRep_ServerYaw)
	float ServerYaw = 0.0f;
	UPROPERTY(ReplicatedUsing=OnRep_ServerPitch)
	float ServerPitch = 0.0f;

	float LocalYaw = 0.0f;
	float LocalPitch = 0.0f;

	float TargetYaw = 0.0f;
	float TargetPitch = 0.0f;

	UPROPERTY(ReplicatedUsing=OnRep_CurrentOperator)
	TWeakObjectPtr<AActor> CurrentOperator;

	FRotator InitialBaseRotation;
	FRotator InitialBarrelRotation;

	float LastFireTime = 0.0f;

	FTimerHandle InterpTimerHandle;
};