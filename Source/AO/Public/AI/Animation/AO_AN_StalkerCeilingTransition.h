//KSJ : AO_AN_StalkerCeilingTransition - Stalker 천장/바닥 전환 AnimNotify

#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimNotifies/AnimNotify.h"
#include "AO_AN_StalkerCeilingTransition.generated.h"

/**
 * Stalker 천장/바닥 전환 AnimNotify
 * 
 * 몽타주의 특정 프레임에서 천장/바닥 모드를 전환합니다.
 * 점프 몽타주의 "천장 접촉 순간" 또는 "바닥 착지 순간"에 배치하세요.
 * 
 * 사용법:
 * 1. JumpToCeiling 몽타주: 천장에 접촉하는 프레임에 추가, bToCeiling = true
 * 2. JumpToFloor 몽타주: 바닥에 착지하는 프레임에 추가, bToCeiling = false
 */
UCLASS(meta = (DisplayName = "AO Stalker Ceiling Transition"))
class AO_API UAO_AN_StalkerCeilingTransition : public UAnimNotify
{
	GENERATED_BODY()

public:
	UAO_AN_StalkerCeilingTransition();

	virtual void Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, 
	                    const FAnimNotifyEventReference& EventReference) override;

	virtual FString GetNotifyName_Implementation() const override;

	// true: 천장 모드로 전환 (바닥→천장 점프 몽타주에서 사용)
	// false: 바닥 모드로 전환 (천장→바닥 점프 몽타주에서 사용)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ceiling Transition")
	bool bToCeiling = true;
};

