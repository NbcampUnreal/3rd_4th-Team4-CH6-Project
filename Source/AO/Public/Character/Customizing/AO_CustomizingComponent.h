// AO_CustomizingComponent.h

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "AO_CustomizingComponent.generated.h"

class AAO_PlayerState;
class AAO_PlayerCharacter;
class UCustomizableObject;
class UCustomizableObjectInstance;

USTRUCT(BlueprintType)
struct FParameterOptionName
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString ParameterName;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString OptionName;
};

UENUM(BlueprintType)
enum class ECharacterMesh : uint8
{
	Elsa UMETA(DisplayName = "Elsa"),
	Anka UMETA(DisplayName = "Anka"),
};

USTRUCT(BlueprintType)
struct FCustomizingData
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	ECharacterMesh CharacterMeshType = ECharacterMesh::Elsa;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FParameterOptionName HairOptionData;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FParameterOptionName ClothOptionData;
};

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class AO_API UAO_CustomizingComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	UAO_CustomizingComponent();
	
	UFUNCTION(Server, Reliable)
	void ServerRPC_ChangeCustomizing(const FCustomizingData& NewCustomizingData);
	
	TObjectPtr<UCustomizableObjectInstance> GetCustomizableObjectInstanceFromMap(ECharacterMesh MeshType) const;

	const FCustomizingData& GetCustomizingData() const;

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
	
	UPROPERTY(ReplicatedUsing = OnRep_CustomizingData)
	FCustomizingData CustomizingData;
	
	UFUNCTION()
	void OnRep_CustomizingData();
	
	void ChangeCharacterMesh(UCustomizableObjectInstance* Instance);
	void ChangeOption(UCustomizableObjectInstance* Instance, const FParameterOptionName& NewOptionData);

	void LoadCustomizingDataFromPlayerState();
	
	void ApplyCustomizingData();
};