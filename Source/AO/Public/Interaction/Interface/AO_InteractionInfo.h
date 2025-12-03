// HSJ : AO_InteractionInfo.h
#pragma once

#include "Abilities/GameplayAbility.h"
#include "AO_InteractionInfo.generated.h"

class IAO_Interface_Interactable;

/**
 * 상호작용에 필요한 모든 정보를 담는 핵심 구조체
 * 상호작용 가능한 객체가 플레이어에게 제공하는 상호작용의 세부사항을 정의
 * 주요 구성 요소:
 * - UI 표시 정보 (제목, 설명)
 * - 홀딩 시스템 (지속시간)
 * - 어빌리티 시스템 연동 (실행할 어빌리티)
 * - 애니메이션  (몽타주)
 * - 커스텀 UI 위젯
 */
USTRUCT(BlueprintType)
struct FAO_InteractionInfo
{
	GENERATED_BODY()

public:
	// 이 상호작용 정보를 제공하는 상호작용 가능한 객체 참조
	UPROPERTY(BlueprintReadWrite)
	TScriptInterface<IAO_Interface_Interactable> Interactable;

	// 상호작용 UI에 표시될 제목
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FText Title;

	// 상호작용 UI에 표시될 상세 설명
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FText Content;

	// 홀딩 상호작용의 지속시간
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float Duration = 0.f;

	// 상호작용 실행 시 플레이어에게 부여될 어빌리티 클래스
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	TSubclassOf<UGameplayAbility> AbilityToGrant;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Animation")
	TObjectPtr<UAnimMontage> ActiveHoldMontage;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Animation")
	TObjectPtr<UAnimMontage> ActiveMontage;

	// Motion Warping용 Transform (손이 가야 할 위치)
	UPROPERTY(BlueprintReadWrite)
	FTransform InteractionTransform;
    
	// 몽타주 내 Warp Target 이름 (InteractionPoint)
	UPROPERTY(BlueprintReadWrite)
	FName WarpTargetName = NAME_None;

	// 모든 멤버 변수를 비교하여 완전히 동일한 경우에만 true 반환, 상호작용 정보 변화 감지에 사용
	FORCEINLINE bool operator==(const FAO_InteractionInfo& Other) const
	{
		return Interactable == Other.Interactable &&
			Title.IdenticalTo(Other.Title) &&
			Content.IdenticalTo(Other.Content) &&
			Duration == Other.Duration &&
			AbilityToGrant == Other.AbilityToGrant &&
			ActiveHoldMontage == Other.ActiveHoldMontage &&
			ActiveMontage == Other.ActiveMontage;
	}

	FORCEINLINE bool operator!=(const FAO_InteractionInfo& Other) const
	{
		return !operator==(Other);
	}
};