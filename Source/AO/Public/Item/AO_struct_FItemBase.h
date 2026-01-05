#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "AO_struct_FItemBase.generated.h"

UENUM(BlueprintType)
enum class EItemType : uint8
{
	None,
	Fuel,
	Consumable,
	Weapon,
	Passive,
	RevivalChip
};

USTRUCT(BlueprintType)
struct AO_API FAO_struct_FItemBase : public FTableRowBase
{
	GENERATED_BODY()

public:
	FAO_struct_FItemBase();

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	EItemType ItemType = EItemType::None;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FName ItemID;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString ItemName;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float FuelAmount = 0.f;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TSoftObjectPtr<UTexture2D> ItemIcon;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TSoftObjectPtr<UStaticMesh> WorldMesh;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 ItemPrice = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 PassiveAmount = 0;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FGameplayTagContainer ItemTags;
};

inline FAO_struct_FItemBase::FAO_struct_FItemBase()
{
}
