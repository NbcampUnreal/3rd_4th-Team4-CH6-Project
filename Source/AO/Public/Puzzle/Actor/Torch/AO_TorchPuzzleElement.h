// HSJ : AO_TorchPuzzleElement.h
#pragma once

#include "CoreMinimal.h"
#include "Puzzle/Element/AO_PuzzleElement.h"
#include "AO_TorchPuzzleElement.generated.h"

UCLASS()
class AO_API AAO_TorchPuzzleElement : public AAO_PuzzleElement
{
	GENERATED_BODY()

public:
	AAO_TorchPuzzleElement(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	virtual void OnInteractionSuccess(AActor* Interactor) override;

protected:
	virtual void BeginPlay() override;

private:
	void CheckAndBroadcastCorrectState();

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Torch", Replicated)
	bool bIsTorchLit = true;
	
	// 정답 상태
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Torch")
	bool bCorrectStateIsLit = true;
    
	// 체커에게 보낼 정답 태그
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Torch")
	FGameplayTag TorchCorrectTag;
};