// HSJ : AO_PressurePlate.h
#pragma once

#include "CoreMinimal.h"
#include "Puzzle/Element/AO_PuzzleElement.h"
#include "AO_PressurePlate.generated.h"

class UBoxComponent;

/**
 * Overlap 기반 압력판
 * 
 * - 플레이어가 올라가면 활성화, 내려가면 비활성화
 * - 여러 명이 올라가 있을 때 모두 내려가야 비활성화
 */
UCLASS()
class AO_API AAO_PressurePlate : public AAO_PuzzleElement
{
	GENERATED_BODY()

public:
	AAO_PressurePlate(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

	virtual bool CanInteraction(const FAO_InteractionQuery& InteractionQuery) const override;
	virtual void ResetToInitialState() override;

protected:
	virtual void BeginPlay() override;

	UFUNCTION()
	void OnOverlapBegin(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, 
		UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

	UFUNCTION()
	void OnOverlapEnd(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, 
		UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Components")
	TObjectPtr<UBoxComponent> OverlapTrigger;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="PressurePlate")
	FVector TriggerExtent = FVector(50.f, 50.f, 20.f);

	// Overlap 중인 액터들 추적
	UPROPERTY()
	TArray<TObjectPtr<AActor>> OverlappingActors;
};