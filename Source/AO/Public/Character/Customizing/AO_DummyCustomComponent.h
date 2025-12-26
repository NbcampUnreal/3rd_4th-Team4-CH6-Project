// AO_DummyCustomComponent.h

#pragma once

#include "CoreMinimal.h"
#include "AO_CustomizingComponent.h"
#include "Components/ActorComponent.h"
#include "AO_DummyCustomComponent.generated.h"

class AAO_CustomizingCharacter;
class AAO_PlayerState;
class UCustomizableObject;
class UCustomizableObjectInstance;


UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class AO_API UAO_DummyCustomComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	UAO_DummyCustomComponent();
	
	UFUNCTION(BlueprintCallable, Category = "Customizing")
	UCustomizableObjectInstance* GetCurrentCustomizableObjectInstanceFromMap() const;
	
	UFUNCTION(BlueprintCallable, Category = "Customizing")
	void SaveCustomizingData();

	UFUNCTION(BlueprintCallable, Category = "Customizing")
	void ChangeCharacterMeshInBlueprint(ECharacterMesh NewMeshType);

	UFUNCTION(BlueprintCallable, Category = "Customizing")
	void ChangeOptionInBlueprint(const FParameterOptionName& NewOptionData);
	
protected:
	virtual void BeginPlay() override;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Customizing")
	TMap<ECharacterMesh, TObjectPtr<UCustomizableObject>> CustomizableObjectMap;

private:
	UPROPERTY()
	TObjectPtr<AAO_CustomizingCharacter> CustomizingCharacter;
	UPROPERTY()
	TMap<ECharacterMesh, TObjectPtr<UCustomizableObjectInstance>> CustomizableObjectInstanceMap;

	FCustomizingData CustomizingData;
	
	void ChangeCharacterMesh(UCustomizableObjectInstance* Instance);
	void ChangeOption(UCustomizableObjectInstance* Instance, const FParameterOptionName& NewOptionData);
	
	void ApplyCustomizingData();
};