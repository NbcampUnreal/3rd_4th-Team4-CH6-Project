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

	UFUNCTION(BlueprintCallable, Server, Reliable, Category = "Customizing")
	void ServerRPC_ChangeCharacter(ECharacterMesh MeshType);
	
	UFUNCTION(BlueprintCallable, Server, Reliable, Category = "Customizing")
	void ServerRPC_ChangeCustomizingOption(FParameterOptionName NewOptionData);
	
	UFUNCTION(BlueprintCallable, Category = "Customizing")
	UCustomizableObjectInstance* GetCustomizableObjectInstanceFromMap(ECharacterMesh MeshType) const;
	
	UFUNCTION(BlueprintCallable, Category = "Customizing")
	ECharacterMesh GetCurrentMeshType() const;

	UFUNCTION(BlueprintCallable, Category = "Customizing")
	void SaveCustomizingDataToPlayerState();
	void LoadCustomizingDataFromPlayerState();
	
	void PrintCustomizableObjectInstanceMap();
	void PrintPSCustomizingData();
	
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
	
	UPROPERTY(ReplicatedUsing = OnRep_CurrentMeshType)
	ECharacterMesh CurrentMeshType = ECharacterMesh::Elsa;
	UPROPERTY(ReplicatedUsing = OnRep_CurrentOptionData)
	FParameterOptionName CurrentOptionData;

	FCustomizingData CustomizingData;

	UFUNCTION()
	void OnRep_CurrentMeshType();
	UFUNCTION()
	void OnRep_CurrentOptionData();
	
	void ChangeCharacterMesh();
	void ChangeOption();
	
	void ApplyCustomizingData();
	
};
