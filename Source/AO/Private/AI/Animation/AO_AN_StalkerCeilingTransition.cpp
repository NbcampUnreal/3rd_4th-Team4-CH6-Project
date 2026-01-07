//KSJ : AO_AN_StalkerCeilingTransition - Stalker 천장/바닥 전환 AnimNotify

#include "AI/Animation/AO_AN_StalkerCeilingTransition.h"
#include "AI/Character/AO_Stalker.h"

UAO_AN_StalkerCeilingTransition::UAO_AN_StalkerCeilingTransition()
{
	bToCeiling = true;
}

void UAO_AN_StalkerCeilingTransition::Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, 
                                              const FAnimNotifyEventReference& EventReference)
{
	Super::Notify(MeshComp, Animation, EventReference);

	if (!MeshComp)
	{
		return;
	}

	// 에디터 프리뷰에서는 실행하지 않음
	if (UWorld* World = MeshComp->GetWorld())
	{
		if (!World->IsGameWorld())
		{
			return;
		}
	}

	// Stalker 캐릭터 가져오기
	AAO_Stalker* Stalker = Cast<AAO_Stalker>(MeshComp->GetOwner());
	if (!Stalker)
	{
		return;
	}

	// 천장/바닥 모드 전환
	Stalker->SetCeilingMode(bToCeiling);
}

FString UAO_AN_StalkerCeilingTransition::GetNotifyName_Implementation() const
{
	return bToCeiling ? TEXT("Stalker: To Ceiling") : TEXT("Stalker: To Floor");
}

