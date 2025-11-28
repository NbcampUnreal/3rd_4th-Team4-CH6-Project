#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "AbilitySystemComponent.h"
#include "AbilitySystemInterface.h"
#include "AO_PassiveContainer.generated.h"

UCLASS()
class AO_API AAO_PassiveContainer : public AActor, public IAbilitySystemInterface
{
	GENERATED_BODY()

public:
	AAO_PassiveContainer();

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Mesh")
	TObjectPtr<UStaticMeshComponent> StaticMesh;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Interact")
	class UAO_InteractableComponent* InteractableComp;

	virtual UAbilitySystemComponent* GetAbilitySystemComponent() const;
	
	UFUNCTION()
	void HandleInteractionSuccess(AActor* Interactor);
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "GAS")
	TObjectPtr<UAbilitySystemComponent> ASC;

	UPROPERTY(Replicated, EditAnywhere, BlueprintReadOnly, Category="Item")
	UDataTable* ItemDataTable;

protected:
	virtual void BeginPlay() override;

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

};
