// AO_AnimNotify_SendDeathRagdollEvent.h

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "Animation/AnimNotifies/AnimNotify.h"
#include "AO_AnimNotify_SendDeathRagdollEvent.generated.h"

UCLASS()
class AO_API UAO_AnimNotify_SendDeathRagdollEvent : public UAnimNotify
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "GAS")
	FGameplayTag RagdollEventTag;

	virtual void Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, const FAnimNotifyEventReference& EventReference) override;
};
