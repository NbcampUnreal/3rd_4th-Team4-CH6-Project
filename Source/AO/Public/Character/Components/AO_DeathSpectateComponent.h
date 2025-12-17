// AO_DeathSpectateComponent.h

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "AO_DeathSpectateComponent.generated.h"

class AAO_PlayerCharacter;

USTRUCT()
struct FRepCameraView
{
	GENERATED_BODY()

	UPROPERTY()
	FVector_NetQuantize10 Location = FVector::ZeroVector;

	UPROPERTY()
	FRotator Rotation = FRotator::ZeroRotator;

	UPROPERTY()
	float FOV = 90.f;
};

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class AO_API UAO_DeathSpectateComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UAO_DeathSpectateComponent();

protected:
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
	virtual void GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const override;
	
public:
	// 사망 관련 처리
	void BindDeathDelegate();
	bool IsAlive_Server() const;

	// 누군가 관전 시작/종료했다고 등록
	void AddSpectator(APlayerController* SpectatorPC);
	void RemoveSpectator(APlayerController* SpectatorPC);

	bool GetRepCameraView(FRepCameraView& OutView) const;

	void NotifySpectators_TargetInvalidated();

private:
	UFUNCTION()
	void OnOwnerDied();
	UFUNCTION(Client, Reliable)
	void ClientRPC_HandleDeathView();
	UFUNCTION(Server, Unreliable)
	void ServerRPC_UpdateCameraView(const FRepCameraView& NewView);

	void StartCameraSyncTimer();
	void StopCameraSyncTimer();
	void SendCameraViewToServer();

	UPROPERTY(ReplicatedUsing=OnRep_StreamEnabled)
	bool bStreamEnabled = false;

	UFUNCTION()
	void OnRep_StreamEnabled();

	UPROPERTY()
	TSet<TWeakObjectPtr<APlayerController>> SpectatorSet;

	UPROPERTY(Replicated)
	FRepCameraView RepCameraView;

	FTimerHandle TimerHandle_CameraSync;

	UPROPERTY()
	TObjectPtr<AActor> OwnerActor = nullptr;
	UPROPERTY()
	TObjectPtr<AAO_PlayerCharacter> OwnerCharacter = nullptr;
};
