// AO_CustomizingComponent.h

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "AO_CustomizingComponent.generated.h"

class AAO_PlayerCharacter;
class UCustomizableObject;
class UCustomizableObjectInstance;

UENUM(BlueprintType)
enum class ECharacterMesh : uint8
{
	None UMETA(DisplayName = "None"),
	Elsa UMETA(DisplayName = "Elsa"),
	Anka UMETA(DisplayName = "Anka"),
};

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class AO_API UAO_CustomizingComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	UAO_CustomizingComponent();

	UFUNCTION(BlueprintCallable, Server, Reliable, Category = "Customizing")
	void ServerRPC_ChangeCharacter(ECharacterMesh MeshType);
	UFUNCTION()
	void ChangeCharacterMesh(UCustomizableObjectInstance* Instance);

	void PrintCustomizableObjectInstanceMap();
	
	
protected:
	virtual void BeginPlay() override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Customizing")
	TMap<ECharacterMesh, TObjectPtr<UCustomizableObject>> CustomizableObjectMap;

private:
	UPROPERTY()
	TObjectPtr<AAO_PlayerCharacter> PlayerCharacter;
	UPROPERTY()
	TMap<ECharacterMesh, TObjectPtr<UCustomizableObjectInstance>> CustomizableObjectInstanceMap;
	UPROPERTY(ReplicatedUsing = OnRep_MeshType)
	ECharacterMesh CurrentMeshType = ECharacterMesh::None;

	UFUNCTION()
	void OnRep_MeshType();

		
};
