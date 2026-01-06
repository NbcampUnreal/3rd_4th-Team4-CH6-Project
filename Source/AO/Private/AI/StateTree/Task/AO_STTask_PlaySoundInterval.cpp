//KSJ : AO_STTask_PlaySoundInterval

#include "AI/StateTree/Task/AO_STTask_PlaySoundInterval.h"

#include "Components/AudioComponent.h"
#include "Components/SceneComponent.h"
#include "Engine/EngineTypes.h"
#include "Kismet/GameplayStatics.h"
#include "Sound/SoundAttenuation.h"
#include "Sound/SoundBase.h"
#include "StateTreeExecutionContext.h"

AActor* FAO_STTask_PlaySoundInterval::ResolveOwnerActor(FStateTreeExecutionContext& Context)
{
	UObject* OwnerObject = Context.GetOwner();
	AActor* OwnerActor = Cast<AActor>(OwnerObject);
	if (!OwnerActor)
	{
		return nullptr;
	}

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

	return OwnerActor;
}

float FAO_STTask_PlaySoundInterval::ComputeNextInterval(const float MinInterval, const float MaxInterval)
{
	const float ClampedMin = FMath::Max(0.1f, MinInterval);
	const float ClampedMax = FMath::Max(ClampedMin, MaxInterval);
	return FMath::FRandRange(ClampedMin, ClampedMax);
}

void FAO_STTask_PlaySoundInterval::StopAndDestroyAudioComponent(TObjectPtr<UAudioComponent>& InOutAudioComponent)
{
	if (!InOutAudioComponent)
	{
		return;
	}

	if (InOutAudioComponent->IsPlaying())
	{
		InOutAudioComponent->Stop();
	}

	InOutAudioComponent->DestroyComponent();
	InOutAudioComponent = nullptr;
}

EStateTreeRunStatus FAO_STTask_PlaySoundInterval::EnterState(FStateTreeExecutionContext& Context, const FStateTreeTransitionResult& Transition) const
{
	FInstanceDataType& InstanceData = Context.GetInstanceData<FInstanceDataType>(*this);

	AActor* OwnerActor = ResolveOwnerActor(Context);
	if (!OwnerActor)
	{
		return EStateTreeRunStatus::Failed;
	}

	if (!InstanceData.Sound)
	{
		return EStateTreeRunStatus::Failed;
	}

	// Task 시작 시 타이머 초기화
	InstanceData.Timer = ComputeNextInterval(InstanceData.MinInterval, InstanceData.MaxInterval);

	// 겹침 방지 모드면, 재사용 가능한 오디오 컴포넌트를 생성해둠
	if (InstanceData.bPreventOverlap)
	{
		USceneComponent* RootComp = OwnerActor->GetRootComponent();
		if (!RootComp)
		{
			return EStateTreeRunStatus::Failed;
		}

		InstanceData.AudioComponent = NewObject<UAudioComponent>(OwnerActor, TEXT("AO_STTask_PlaySoundInterval_Audio"));
		if (!ensure(InstanceData.AudioComponent))
		{
			return EStateTreeRunStatus::Failed;
		}

		InstanceData.AudioComponent->SetupAttachment(RootComp, InstanceData.AttachPointName);
		InstanceData.AudioComponent->RegisterComponent();

		InstanceData.AudioComponent->SetSound(InstanceData.Sound);
		InstanceData.AudioComponent->SetVolumeMultiplier(InstanceData.VolumeMultiplier);
		InstanceData.AudioComponent->SetPitchMultiplier(InstanceData.PitchMultiplier);

		if (InstanceData.AttenuationSettings)
		{
			InstanceData.AudioComponent->AttenuationSettings = InstanceData.AttenuationSettings;
		}

		InstanceData.AudioComponent->bAutoDestroy = false;
	}

	// Enter에서 즉시 1회 재생 옵션
	if (InstanceData.bPlayOnEnter)
	{
		// 즉시 재생 후 다음 타이머를 다시 랜덤 설정
		InstanceData.Timer = 0.f;
	}

	return EStateTreeRunStatus::Running;
}

EStateTreeRunStatus FAO_STTask_PlaySoundInterval::Tick(FStateTreeExecutionContext& Context, const float DeltaTime) const
{
	FInstanceDataType& InstanceData = Context.GetInstanceData<FInstanceDataType>(*this);

	AActor* OwnerActor = ResolveOwnerActor(Context);
	if (!OwnerActor)
	{
		return EStateTreeRunStatus::Failed;
	}

	if (!InstanceData.Sound)
	{
		return EStateTreeRunStatus::Failed;
	}

	InstanceData.Timer -= DeltaTime;
	if (InstanceData.Timer > 0.f)
	{
		return EStateTreeRunStatus::Running;
	}

	// 다음 간격 계산을 먼저 해두어 Tick 호출 타이밍에 따른 편차를 줄임
	InstanceData.Timer = ComputeNextInterval(InstanceData.MinInterval, InstanceData.MaxInterval);

	// 겹침 방지: 재생 중이면 이번 트리거는 스킵
	if (InstanceData.bPreventOverlap)
	{
		if (!InstanceData.AudioComponent)
		{
			return EStateTreeRunStatus::Failed;
		}

		if (InstanceData.AudioComponent->IsPlaying())
		{
			return EStateTreeRunStatus::Running;
		}

		// 재생 (이미 컴포넌트에 Sound 설정됨)
		InstanceData.AudioComponent->Play(0.f);
		return EStateTreeRunStatus::Running;
	}

	// 겹침 허용: 매번 새로 스폰 (AutoDestroy)
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
			true, // bStopWhenAttachedToDestroyed
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
		// 부착하지 않는 경우: 위치 기반으로 1회 스폰
		// SpawnSoundAtLocation은 AutoDestroy 설정 가능 (UE 5.6 시그니처 확인)
		// - WorldContextObject로 OwnerActor 사용
		UAudioComponent* TempComp = UGameplayStatics::SpawnSoundAtLocation(
			OwnerActor,
			InstanceData.Sound,
			OwnerActor->GetActorLocation(),
			FRotator::ZeroRotator,
			InstanceData.VolumeMultiplier,
			InstanceData.PitchMultiplier,
			0.f,
			InstanceData.AttenuationSettings,
			nullptr,
			true
		);

		if (TempComp)
		{
			TempComp->bAutoDestroy = true;
		}
	}

	return EStateTreeRunStatus::Running;
}

void FAO_STTask_PlaySoundInterval::ExitState(FStateTreeExecutionContext& Context, const FStateTreeTransitionResult& Transition) const
{
	FInstanceDataType& InstanceData = Context.GetInstanceData<FInstanceDataType>(*this);

	// Task 종료 시 타이머 리셋
	InstanceData.Timer = 0.f;

	if (InstanceData.bStopOnExit)
	{
		StopAndDestroyAudioComponent(InstanceData.AudioComponent);
	}
	else
	{
		// Stop하지 않더라도, Task가 만든 컴포넌트는 정리해 Dangling을 방지
		if (InstanceData.AudioComponent)
		{
			InstanceData.AudioComponent->DestroyComponent();
			InstanceData.AudioComponent = nullptr;
		}
	}
}


