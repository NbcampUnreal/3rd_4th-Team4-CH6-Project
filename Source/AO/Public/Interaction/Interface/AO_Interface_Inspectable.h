// HSJ : AO_Interface_Inspectable.h
#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "AO_Interface_Inspectable.generated.h"

UINTERFACE(MinimalAPI, BlueprintType)
class UAO_Interface_Inspectable : public UInterface
{
	GENERATED_BODY()
};

/**
 * Inspection 중 메시 클릭 처리가 가능한 액터 인터페이스
 */
class AO_API IAO_Interface_Inspectable
{
	GENERATED_BODY()

public:
	// Inspection 중 메시 클릭 처리
	virtual void OnInspectionMeshClicked(UPrimitiveComponent* ClickedComponent) = 0;
};