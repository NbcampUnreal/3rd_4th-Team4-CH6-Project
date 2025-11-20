// HSJ : AO_InspectableComponent.h
#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "AO_InspectableComponent.generated.h"

/**
 * Inspection 가능한 오브젝트에 부착되는 컴포넌트
 * 
 * 1. Inspection 카메라 위치/회전 정보 제공
 * 2. Lock 상태 관리 (한 번에 한 명만 검사 가능)
 * 
 */

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class AO_API UAO_InspectableComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UAO_InspectableComponent();

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	UFUNCTION(BlueprintPure, Category = "Inspection")
	FTransform GetInspectionCameraTransform() const;

	void SetInspectionLocked(bool bLocked, AActor* InspectingPlayer);

	UFUNCTION(BlueprintPure, Category = "Inspection")
	bool IsLockedByOtherPlayer(AActor* QueryPlayer) const;

public:
	UPROPERTY(EditAnywhere, Category = "Inspection|Camera")
	FVector CameraRelativeLocation = FVector(100.0f, 0.0f, 50.0f);

	UPROPERTY(EditAnywhere, Category = "Inspection|Camera")
	FRotator CameraRelativeRotation = FRotator(-10.0f, 180.0f, 0.0f);

protected:
	UPROPERTY(Replicated)
	bool bIsLocked = false;

	UPROPERTY(Replicated)
	TObjectPtr<AActor> CurrentInspectingPlayer;
};