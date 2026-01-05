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
	FString ItemExplain;

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

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Interaction")
	uint8 HighlightStencilValue = 0;

	uint8 GetHighlightStencilValue() const
	{
		if (HighlightStencilValue > 0)
		{
			return HighlightStencilValue;
		}

		switch(ItemType)
		{
		case EItemType::Fuel:        return 251;
		case EItemType::Consumable:  return 252;
		case EItemType::Weapon:      return 253;
		case EItemType::Passive:     return 254;
		case EItemType::RevivalChip: return 255;
		default:                     return 250;
		}
	}
};

inline FAO_struct_FItemBase::FAO_struct_FItemBase()
{
}
