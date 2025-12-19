// UAO_PlayerCharacter_AttributeDefaults.h

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "AO_PlayerCharacter_AttributeDefaults.generated.h"

UCLASS()
class AO_API UAO_PlayerCharacter_AttributeDefaults : public UDataAsset
{
	GENERATED_BODY()

public:
	// 체력
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Health")
	float Health = 100.0f;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Health")
	float MaxHealth = 100.0f;

	// 스태미나
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Stamina")
	float Stamina = 100.0f;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Stamina")
	float MaxStamina = 100.0f;

	// 이동속도
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Speed")
	float WalkSpeed = 200.0f;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Speed")
	float RunSpeed = 500.0f;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Speed")
	float SprintSpeed = 800.0f;
};
