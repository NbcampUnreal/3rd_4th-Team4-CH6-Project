// HSJ : AO_PinballBall.h
#pragma once
#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "AO_PinballBall.generated.h"

UCLASS()
class AO_API AAO_PinballBall : public AActor
{
	GENERATED_BODY()

public:
	AAO_PinballBall();
    
	UFUNCTION(BlueprintCallable)
	void ResetToStart();

protected:
	virtual void BeginPlay() override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

public:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	TObjectPtr<UStaticMeshComponent> BallMesh;

protected:
	UPROPERTY(Replicated)
	FVector StartPosition;
};