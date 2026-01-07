//KSJ : AO_STTask_PlaySoundOnce

#include "AI/StateTree/Task/AO_STTask_PlaySoundOnce.h"

#include "Components/SceneComponent.h"
#include "Engine/EngineTypes.h"
#include "Kismet/GameplayStatics.h"
#include "Sound/SoundAttenuation.h"
#include "Sound/SoundBase.h"
#include "StateTreeExecutionContext.h"

AActor* FAO_STTask_PlaySoundOnce::ResolveOwnerActor(FStateTreeExecutionContext& Context)
{
	UObject* OwnerObject = Context.GetOwner();
	AActor* OwnerActor = Cast<AActor>(OwnerObject);
	if (!OwnerActor)
	{
		return nullptr;
	}

	// Controller가 Owner로 들어오는 경우 Pawn을 우선 사용
	AController* AsController = Cast<AController>(OwnerActor);
	if (AsController)
	{
		APawn* Pawn = AsController->GetPawn();
		if (Pawn)
		{
			return Pawn;
		}
		return AsController;
	}

	// Pawn/Actor인 경우 그대로 사용
	return OwnerActor;
}

EStateTreeRunStatus FAO_STTask_PlaySoundOnce::EnterState(FStateTreeExecutionContext& Context, const FStateTreeTransitionResult& Transition) const
{
	const FInstanceDataType& InstanceData = Context.GetInstanceData<FInstanceDataType>(*this);

	AActor* OwnerActor = ResolveOwnerActor(Context);
	if (!OwnerActor)
	{
		return EStateTreeRunStatus::Failed;
	}

	if (!InstanceData.Sound)
	{
		// 설정 누락은 조용히 무시하지 않고 실패로 처리 (디자이너 세팅 오류 빠른 발견)
		return EStateTreeRunStatus::Failed;
	}

	if (InstanceData.bAttachToOwner)
	{
		USceneComponent* RootComp = OwnerActor->GetRootComponent();
		if (!RootComp)
		{
			return EStateTreeRunStatus::Failed;
		}

		// UE 5.6 시그니처 확인 완료 (GameplayStatics.h)
		UGameplayStatics::SpawnSoundAttached(
			InstanceData.Sound,
			RootComp,
			InstanceData.AttachPointName,
			FVector(ForceInit),
			FRotator::ZeroRotator,
			EAttachLocation::KeepRelativeOffset,
			true, // bStopWhenAttachedToDestroyed: Owner 파괴 시 자동 정지
			InstanceData.VolumeMultiplier,
			InstanceData.PitchMultiplier,
			0.f,
			InstanceData.AttenuationSettings,
			nullptr,
			true // bAutoDestroy
		);
	}
	else
	{
		const FVector Location = OwnerActor->GetActorLocation();

		UGameplayStatics::SpawnSoundAtLocation(
			OwnerActor,
			InstanceData.Sound,
			Location,
			FRotator::ZeroRotator,
			InstanceData.VolumeMultiplier,
			InstanceData.PitchMultiplier,
			0.f,
			InstanceData.AttenuationSettings,
			nullptr,
			true // bAutoDestroy
		);
	}

	return EStateTreeRunStatus::Succeeded;
}


