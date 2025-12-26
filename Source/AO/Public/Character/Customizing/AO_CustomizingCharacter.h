//AO_CustomizingCharacter.h

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "AO_CustomizingCharacter.generated.h"

class UAO_DummyCustomComponent;
class USpringArmComponent;
class UCameraComponent;
class UCustomizableSkeletalComponent;

UCLASS()
class AO_API AAO_CustomizingCharacter : public AActor
{
	GENERATED_BODY()

public:
	AAO_CustomizingCharacter();

protected:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "PlayerCharacter|Components")
	TObjectPtr<USceneComponent> Root;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "PlayerCharacter|Components")
	TObjectPtr<USpringArmComponent> SpringArm;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "PlayerCharacter|Components")
	TObjectPtr<UCameraComponent> Camera;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "PlayerCharacter|Components")
	TObjectPtr<USkeletalMeshComponent> DefaultSkeletalMesh;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "PlayerCharacter|Components")
	TObjectPtr<USkeletalMeshComponent> BaseSkeletalMesh;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "PlayerCharacter|Components")
	TObjectPtr<USkeletalMeshComponent> BodySkeletalMesh;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "PlayerCharacter|Components")
	TObjectPtr<USkeletalMeshComponent> HeadSkeletalMesh;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "PlayerCharacter|Components")
	TObjectPtr<UCustomizableSkeletalComponent> BodyComponent;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "PlayerCharacter|Components")
	TObjectPtr<UCustomizableSkeletalComponent> HeadComponent;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "PlayerCharacter|Components")
	TObjectPtr<UAO_DummyCustomComponent> DummyCustomComponent;

public:
	TObjectPtr<UCustomizableSkeletalComponent> GetBodyComponent() const { return BodyComponent; }
	TObjectPtr<UCustomizableSkeletalComponent> GetHeadComponent() const { return HeadComponent; }
	TObjectPtr<UAO_DummyCustomComponent> GetCustomizingComponent() const { return DummyCustomComponent; }
};
