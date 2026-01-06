// AO_CustomizingComponent.h

#pragma once

#include "CoreMinimal.h"
#include "AO_DelegateManager.h"
#include "Components/ActorComponent.h"
#include "AO_CustomizingComponent.generated.h"

struct FUpdateContext;
class AAO_PlayerState;
class AAO_PlayerCharacter;
class UCustomizableObject;
class UCustomizableObjectInstance;
class UAO_PlayerSoundDataAsset;

DECLARE_MULTICAST_DELEGATE_OneParam(FOnCharacterCapsuleChanged, float /*NewCapsuleHalfHeight*/);

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
	Bruce UMETA(DisplayName = "Bruce"),
	Cameron UMETA(DisplayName = "Cameron"),
};

USTRUCT(BlueprintType)
struct FCustomizingData
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	ECharacterMesh CharacterMeshType = ECharacterMesh::Elsa;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float CapsuleHalfHeight = 81.0f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	USkeletalMesh* CharacterSkeletalMesh = nullptr;
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

	FOnCharacterCapsuleChanged OnCapsuleChangedDelegate;

protected:
	virtual void BeginPlay() override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Customizing")
	TMap<ECharacterMesh, TObjectPtr<UCustomizableObject>> CustomizableObjectMap;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Customizing")
	TMap<ECharacterMesh, TObjectPtr<USkeletalMesh>> SkeletalMeshMap;

private:
	UPROPERTY()
	TObjectPtr<AAO_PlayerCharacter> PlayerCharacter;
	UPROPERTY()
	TMap<ECharacterMesh, TObjectPtr<UCustomizableObjectInstance>> CustomizableObjectInstanceMap;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Customizing", meta = (AllowPrivateAccess = "true"), ReplicatedUsing = OnRep_CustomizingData)
	FCustomizingData CustomizingData;
	
	UFUNCTION()
	void OnRep_CustomizingData();
	
	void ChangeCharacterMesh(UCustomizableObjectInstance* Instance);
	void ChangeOption(UCustomizableObjectInstance* Instance, const FParameterOptionName& NewOptionData);

	void LoadCustomizingDataFromPlayerState();
	
	void ApplyCustomizingData();

	UFUNCTION()
	void OnMeshUpdateFinished(const FUpdateContext& Context);
};