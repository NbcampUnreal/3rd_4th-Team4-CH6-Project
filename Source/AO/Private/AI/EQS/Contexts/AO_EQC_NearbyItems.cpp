// KSJ : AO_EQC_NearbyItems.cpp

#include "AI/EQS/Contexts/AO_EQC_NearbyItems.h"
#include "EnvironmentQuery/EnvQueryTypes.h"
#include "EnvironmentQuery/Items/EnvQueryItemType_Actor.h"
#include "AI/Item/AO_PickupComponent.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/Actor.h"
#include "AO_Log.h"
#include "Engine/OverlapResult.h"

UAO_EQC_NearbyItems::UAO_EQC_NearbyItems()
{
}

void UAO_EQC_NearbyItems::ProvideContext(FEnvQueryInstance& QueryInstance, FEnvQueryContextData& ContextData) const
{
	AActor* QuerierActor = Cast<AActor>(QueryInstance.Owner.Get());
	if (!QuerierActor) return;

	// Querier의 위치 (AI 캐릭터 기준)
	FVector CenterLocation = QuerierActor->GetActorLocation();
	if (APawn* Pawn = Cast<APawn>(QuerierActor))
	{
		CenterLocation = Pawn->GetActorLocation();
	}
	else if (AController* Controller = Cast<AController>(QuerierActor))
	{
		if (APawn* P = Controller->GetPawn())
		{
			CenterLocation = P->GetActorLocation();
		}
	}

	TArray<AActor*> ResultActors;
	
	// 1. Overlap으로 주변 액터 탐색 (효율성)
	TArray<FOverlapResult> Overlaps;
	FCollisionQueryParams Params;
	Params.AddIgnoredActor(QuerierActor);

	// WorldStatic, WorldDynamic 등 아이템이 속할만한 채널 검사
	// (프로젝트 설정에 따라 아이템 전용 채널이 있다면 그걸 쓰는 게 가장 좋음. 여기선 포괄적으로 검사)
	bool bHit = QuerierActor->GetWorld()->OverlapMultiByObjectType(
		Overlaps,
		CenterLocation,
		FQuat::Identity,
		FCollisionObjectQueryParams(FCollisionObjectQueryParams::InitType::AllDynamicObjects),
		FCollisionShape::MakeSphere(SearchRadius),
		Params
	);

	if (bHit)
	{
		for (const FOverlapResult& Overlap : Overlaps)
		{
			AActor* HitActor = Overlap.GetActor();
			if (!HitActor) continue;

			// 2. PickupComponent 검사
			UAO_PickupComponent* PickupComp = HitActor->FindComponentByClass<UAO_PickupComponent>();
			if (!PickupComp) continue;

			// 3. 조건 검사: 이미 들려있는가?
			if (PickupComp->IsPickedUp()) continue;

			// 4. 조건 검사: 무시 태그가 있는가?
			if (PickupComp->HasTag(FGameplayTag::RequestGameplayTag(FName("Item.State.Ignored")))) continue;

			// 통과되면 결과에 추가
			ResultActors.AddUnique(HitActor);
		}
	}

	// EQS에 결과 전달
	if (ResultActors.Num() > 0)
	{
		UEnvQueryItemType_Actor::SetContextHelper(ContextData, ResultActors);
	}
}

