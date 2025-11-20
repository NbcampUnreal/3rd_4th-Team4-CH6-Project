// HSJ : AO_InteractionGameplayAbility.h
#pragma once

#include "CoreMinimal.h"
#include "Abilities/GameplayAbility.h"
#include "AO_InteractionGameplayAbility.generated.h"

/**
 * - Manual: 수동으로만 활성화 (직접 TryActivateAbility 호출)
 * - OnInputTriggered: 입력 트리거 시 활성화
 * - WhileInputActive: 입력이 눌려있는 동안 활성화 유지
 * - OnSpawn: 어빌리티 부여 즉시 자동 활성화
 */
UENUM(BlueprintType)
enum class ECYAbilityActivationPolicy : uint8
{
	Manual,
	OnInputTriggered,
	WhileInputActive,
	OnSpawn,
};

/**
 * 상호작용 어빌리티의 베이스 클래스
 * 
 * - ActivationPolicy: 어빌리티 활성화 시점 제어
 * - OnSpawn 정책: 부여 즉시 자동 실행 (예: 주변 탐색 태스크)
 * - GetControllerFromActorInfo: Owner 체인을 따라 Controller 탐색
 * 
 * 사용 예:
 * - GA_Interact: OnSpawn으로 주변 상호작용 오브젝트 탐색
 * - GA_Interact_Execute: Manual로 F키 입력 시에만 실행
 */
UCLASS()
class AO_API UAO_InteractionGameplayAbility : public UGameplayAbility
{
	GENERATED_BODY()

public:
	UAO_InteractionGameplayAbility();

	virtual void OnAvatarSet(const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilitySpec& Spec) override;

	template<typename T>
	T* GetOwnerAs() const
	{
		return CurrentActorInfo ? Cast<T>(CurrentActorInfo->AvatarActor.Get()) : nullptr;
	}
	
	ECYAbilityActivationPolicy GetActivationPolicy() const { return ActivationPolicy; }

	// Owner 체인을 따라 Controller를 찾아 반환, PlayerController → Controller → Pawn의 Controller 순서로 탐색
	UFUNCTION(BlueprintCallable, Category = "AO|Ability")
	AController* GetControllerFromActorInfo() const;

protected:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "AO|AbilityActivation")
	ECYAbilityActivationPolicy ActivationPolicy;
};