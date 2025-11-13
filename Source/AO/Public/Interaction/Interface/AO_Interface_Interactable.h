// HSJ : AO_Interface_Interactable.h
#pragma once

#include "CoreMinimal.h"
#include "AbilitySystemBlueprintLibrary.h"
#include "AO_InteractionInfo.h"
#include "AO_InteractionQuery.h"
#include "UObject/Interface.h"
#include "AO_Interface_Interactable.generated.h"

/**
 * 상호작용 정보 빌더 헬퍼 클래스
 * 
 * - Interactable로부터 수집된 InteractionInfo에 자동으로 Interactable 참조 추가
 * - GatherPostInteractionInfos에서 사용
 * 
 * 사용 예:
 * FAO_InteractionInfoBuilder Builder(this, OutArray);
 * Builder.AddInteractionInfo(Info); // Info.Interactable = this 자동 설정
 */
class FAO_InteractionInfoBuilder
{
public:
	FAO_InteractionInfoBuilder(TScriptInterface<IAO_Interface_Interactable> InInteractable, TArray<FAO_InteractionInfo>& InInteractionInfos)
		: Interactable(InInteractable)
		, InteractionInfos(InInteractionInfos) 
	{ }

	// InteractionInfo를 배열에 추가하며 Interactable 참조 자동 설정
	void AddInteractionInfo(const FAO_InteractionInfo& InteractionInfo)
	{
		FAO_InteractionInfo& Entry = InteractionInfos.Add_GetRef(InteractionInfo);
		Entry.Interactable = Interactable;
	}

private:
	TScriptInterface<IAO_Interface_Interactable> Interactable;
	TArray<FAO_InteractionInfo>& InteractionInfos;
};

UINTERFACE(MinimalAPI, BlueprintType, meta=(CannotImplementInterfaceInBlueprint))
class UAO_Interface_Interactable : public UInterface
{
	GENERATED_BODY()
};

/**
 * 상호작용 가능한 오브젝트가 구현해야 하는 인터페이스
 * 
 * - GetPreInteractionInfo: 상호작용 정보 제공 (UI 표시용)
 * - GatherPostInteractionInfos: 최종 상호작용 정보 수집 (Duration 보정 포함)
 * - CanInteraction: 현재 상호작용 가능 여부 체크
 * - GetMeshComponents: 하이라이트할 메시 컴포넌트 제공
 * - CustomizeInteractionEventData: GameplayEvent 데이터 커스터마이징 (아직 사용 안함)
 * 
 * 구현 예:
 * - AAO_WorldInteractable: 월드에 배치된 상호작용 오브젝트
 * - AAO_PuzzleElement: 퍼즐 요소
 */
class IAO_Interface_Interactable
{
	GENERATED_BODY()

public:
	// 기본 상호작용 정보 반환, UI 표시 및 상호작용 로직에 사용됨
	virtual FAO_InteractionInfo GetPreInteractionInfo(const FAO_InteractionQuery& InteractionQuery) const 
	{ 
		return FAO_InteractionInfo(); 
	}

	/**
	 * 최종 상호작용 정보 수집 및 보정
	 * 
	 * 1. GetPreInteractionInfo()로 기본 정보 가져오기
	 * 2. Duration을 0 이상으로 보정 (음수 방지)
	 * 3. Interactable 참조 자동 설정
	 * 4. Builder를 통해 결과 배열에 추가
	 */
	void GatherPostInteractionInfos(const FAO_InteractionQuery& InteractionQuery, FAO_InteractionInfoBuilder& InteractionInfoBuilder) const
	{
		FAO_InteractionInfo InteractionInfo = GetPreInteractionInfo(InteractionQuery);
		
		// Duration 보정 (음수 방지)
		if (UAbilitySystemComponent* AbilitySystem = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(InteractionQuery.RequestingAvatar.Get()))
		{
			InteractionInfo.Duration = FMath::Max<float>(0.f, InteractionInfo.Duration);
		}
		
		InteractionInfoBuilder.AddInteractionInfo(InteractionInfo);
	}

	/**
	 * GameplayEvent 데이터 커스터마이징 (선택적 오버라이드)
	 * 
	 * - Execute 어빌리티가 Finalize 이벤트 발생 시 추가 정보 주입
	 * - 예: 퍼즐 요소의 고유 ID, 현재 상태값 등
	 */
	virtual void CustomizeInteractionEventData(const FGameplayTag& InteractionEventTag, FGameplayEventData& InOutEventData) const 
	{ }

	// 하이라이트할 메시 컴포넌트 반환, CustomDepth 렌더링에 사용 (외곽선 표시)
	UFUNCTION(BlueprintCallable)
	virtual void GetMeshComponents(TArray<UMeshComponent*>& OutMeshComponents) const 
	{ }
	
	// 현재 상호작용 가능 여부 체크
	UFUNCTION(BlueprintCallable)
	virtual bool CanInteraction(const FAO_InteractionQuery& InteractionQuery) const 
	{ 
		return true; 
	}
};