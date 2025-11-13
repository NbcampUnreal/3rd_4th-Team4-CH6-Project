// HSJ : AO_InteractionQuery.h
#pragma once

#include "AO_InteractionQuery.generated.h"

/**
 * 상호작용 요청 정보를 담는 구조체
 * 
 * - CanInteraction() 체크 시 요청자 정보 전달
 * - GetInteractionInfo()에서 컨텍스트 제공
 * 
 * 포함 정보:
 * - RequestingAvatar: 상호작용 요청하는 액터 (플레이어 캐릭터)
 * - RequestingController: 요청자의 컨트롤러
 */
USTRUCT(BlueprintType)
struct FAO_InteractionQuery
{
	GENERATED_BODY()

public:
	UPROPERTY(BlueprintReadWrite)
	TWeakObjectPtr<AActor> RequestingAvatar;
	
	UPROPERTY(BlueprintReadWrite)
	TWeakObjectPtr<AController> RequestingController;
};