// HSJ : AO_AbilityTask_GrantNearbyInteraction.h
#pragma once

#include "CoreMinimal.h"
#include "Abilities/Tasks/AbilityTask.h"
#include "AO_AbilityTask_GrantNearbyInteraction.generated.h"

/**
 * 주변 상호작용 오브젝트를 탐색하여 해당 오브젝트의 어빌리티를 동적으로 부여/제거하는 태스크
 * 
 * 1. 일정 주기로 캐릭터 주변을 OverlapSphere로 탐색
 * 2. 감지된 오브젝트의 상호작용 정보(FAO_InteractionInfo) 수집
 * 3. 필요한 어빌리티를 ASC에 동적 부여
 * 4. 범위를 벗어나면 어빌리티 자동 제거
 * 
 * 예시: 플레이어가 상호작용 액터 근처 가면 액터가 설정한 상호작용 어빌리티 부여, 멀어지면 제거
 */
UCLASS()
class AO_API UAO_AbilityTask_GrantNearbyInteraction : public UAbilityTask
{
	GENERATED_BODY()

public:
	// Task 생성
	UFUNCTION(BlueprintCallable, Category="Ability|Tasks", meta=(HidePin="OwningAbility", DefaultToSelf="OwningAbility", BlueprintInternalUseOnly="true"))
	static UAO_AbilityTask_GrantNearbyInteraction* GrantAbilitiesForNearbyInteractables(UGameplayAbility* OwningAbility, float InteractionAbilityScanRange, float InteractionAbilityScanRate);

protected:
	virtual void Activate() override;
	virtual void OnDestroy(bool bInOwnerFinished) override;
	virtual bool IsSupportedForNetworking() const override { return true; }

private:
	// 주변 상호작용 오브젝트 탐색 및 어빌리티 관리
	void QueryInteractables();

	float InteractionAbilityScanRange = 100.f;
	float InteractionAbilityScanRate = 0.1f;

	FTimerHandle QueryTimerHandle;
	
	// 현재 부여된 어빌리티 추적 (어빌리티 클래스 : Handle)
	TMap<FObjectKey, FGameplayAbilitySpecHandle> GrantedInteractionAbilities;
};