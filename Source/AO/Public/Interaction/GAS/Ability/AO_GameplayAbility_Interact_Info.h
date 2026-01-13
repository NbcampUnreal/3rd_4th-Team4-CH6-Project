// HSJ : AO_GameplayAbility_Interact_Info.h
#pragma once

#include "CoreMinimal.h"
#include "AO_InteractionGameplayAbility.h"
#include "Interaction/Interface/AO_InteractionInfo.h"
#include "AO_GameplayAbility_Interact_Info.generated.h"

/**
 * 상호작용 어빌리티의 베이스 클래스
 * 타겟 액터로부터 상호작용 정보를 초기화하고 관리
 */
UCLASS()
class AO_API UAO_GameplayAbility_Interact_Info : public UAO_InteractionGameplayAbility
{
	GENERATED_BODY()

protected:
	// 타겟 액터로부터 상호작용 정보 초기화
	UFUNCTION(BlueprintCallable)
	bool InitializeAbility(AActor* TargetActor);

	UPROPERTY(BlueprintReadOnly)
	TScriptInterface<IAO_Interface_Interactable> Interactable;
	
	UPROPERTY(BlueprintReadOnly)
	TObjectPtr<AActor> InteractableActor;
	
	UPROPERTY(BlueprintReadOnly)
	FAO_InteractionInfo InteractionInfo;
};