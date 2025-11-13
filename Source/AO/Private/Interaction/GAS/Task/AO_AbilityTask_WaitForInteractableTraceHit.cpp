// HSJ : AO_AbilityTask_WaitForInteractableTraceHit.cpp
#include "Interaction/GAS/Task/AO_AbilityTask_WaitForInteractableTraceHit.h"
#include "AbilitySystemComponent.h"
#include "AO_Log.h"
#include "Interaction/Interface/AO_InteractionInfo.h"
#include "Interaction/Interface/AO_Interface_Interactable.h"

UAO_AbilityTask_WaitForInteractableTraceHit::UAO_AbilityTask_WaitForInteractableTraceHit(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
}

UAO_AbilityTask_WaitForInteractableTraceHit* UAO_AbilityTask_WaitForInteractableTraceHit::WaitForInteractableTraceHit(
	UGameplayAbility* OwningAbility, 
	FAO_InteractionQuery InteractionQuery,
	ECollisionChannel TraceChannel, 
	FGameplayAbilityTargetingLocationInfo StartLocation,
	float InteractionTraceRange, 
	float InteractionTraceRate,
	bool bShowDebug, 
	float SphereRadius)
{
	UAO_AbilityTask_WaitForInteractableTraceHit* Task = NewAbilityTask<UAO_AbilityTask_WaitForInteractableTraceHit>(OwningAbility);
	Task->InteractionTraceRange = InteractionTraceRange;
	Task->InteractionTraceRate = InteractionTraceRate;
	Task->StartLocation = StartLocation;
	Task->InteractionQuery = InteractionQuery;
	Task->TraceChannel = TraceChannel;
	Task->bShowDebug = bShowDebug;
	Task->TraceSphereRadius = SphereRadius;
	return Task;
}

void UAO_AbilityTask_WaitForInteractableTraceHit::Activate()
{
	Super::Activate();

	SetWaitingOnAvatar();

	AO_LOG(LogHSJ, Log, TEXT("Trace task activated - Rate: %.2f, Range: %.1f"), 
		InteractionTraceRate, InteractionTraceRange);

	// 주기적 트레이스 시작
	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().SetTimer(TraceTimerHandle, this, &ThisClass::PerformTrace, InteractionTraceRate, true);
	}
}

void UAO_AbilityTask_WaitForInteractableTraceHit::OnDestroy(bool bInOwnerFinished)
{
	// 타이머 정리
	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().ClearTimer(TraceTimerHandle);
	}
	
	Super::OnDestroy(bInOwnerFinished);
}

void UAO_AbilityTask_WaitForInteractableTraceHit::PerformTrace()
{
	AActor* AvatarActor = Ability->GetCurrentActorInfo()->AvatarActor.Get();
	if (!AvatarActor)
	{
		return;
	}
	
	// 트레이스에서 제외할 액터 설정 (자신 + 부착된 액터들)
	TArray<AActor*> ActorsToIgnore;
	ActorsToIgnore.Add(AvatarActor);
	AvatarActor->GetAttachedActors(ActorsToIgnore, false, true);

	FCollisionQueryParams Params(SCENE_QUERY_STAT(UAO_AbilityTask_WaitForInteractableTraceHit), false);
	Params.AddIgnoredActors(ActorsToIgnore);

	// 트레이스 시작점과 종료점 계산
	FVector TraceStart = StartLocation.GetTargetingTransform().GetLocation();
	FVector TraceEnd;
	AimWithPlayerController(AvatarActor, Params, TraceStart, InteractionTraceRange, TraceEnd);

	// SphereCast 수행
	FHitResult HitResult;
	LineTrace(TraceStart, TraceEnd, Params, HitResult);

	// Hit된 오브젝트에서 IAO_Interactable 찾기
	TArray<TScriptInterface<IAO_Interface_Interactable>> Interactables;

	TScriptInterface<IAO_Interface_Interactable> InteractableActor(HitResult.GetActor());
	if (InteractableActor)
	{
		Interactables.AddUnique(InteractableActor);
	}

	TScriptInterface<IAO_Interface_Interactable> InteractableComponent(HitResult.GetComponent());
	if (InteractableComponent)
	{
		Interactables.AddUnique(InteractableComponent);
	}

	// 상호작용 정보 업데이트 (하이라이트, UI)
	UpdateInteractionInfos(InteractionQuery, Interactables);

	// 디버그 드로우 (옵션)
#if ENABLE_DRAW_DEBUG
	if (bShowDebug)
	{
		FColor DebugColor = HitResult.bBlockingHit ? FColor::Red : FColor::Green;
		if (HitResult.bBlockingHit)
		{
			DrawDebugLine(GetWorld(), TraceStart, HitResult.Location, DebugColor, false, InteractionTraceRate);
			DrawDebugSphere(GetWorld(), HitResult.Location, 5.f, 16, DebugColor, false, InteractionTraceRate);
		}
		else
		{
			DrawDebugLine(GetWorld(), TraceStart, TraceEnd, DebugColor, false, InteractionTraceRate);
		}
	}
#endif
}

void UAO_AbilityTask_WaitForInteractableTraceHit::AimWithPlayerController(
	const AActor* InSourceActor, 
	FCollisionQueryParams Params, 
	const FVector& TraceStart, 
	float MaxRange, 
	FVector& OutTraceEnd, 
	bool bIgnorePitch) const
{
	if (!Ability)
	{
		return;
	}
	
	APlayerController* PlayerController = Ability->GetCurrentActorInfo()->PlayerController.Get();
	if (!PlayerController)
	{
		return;
	}
	
	// 카메라 위치와 방향 가져오기
	FVector CameraStart;
	FRotator CameraRotation;
	PlayerController->GetPlayerViewPoint(CameraStart, CameraRotation);

	const FVector CameraDirection = CameraRotation.Vector();
	FVector CameraEnd = CameraStart + (CameraDirection * MaxRange);

	// 카메라 레이를 플레이어 주변 구체 범위 내로 제한
	// (카메라가 멀리 떨어져도 상호작용 범위를 벗어나지 않도록)
	ClipCameraRayToAbilityRange(CameraStart, CameraDirection, TraceStart, MaxRange, CameraEnd);

	// 카메라에서 트레이스하여 장애물 체크
	FHitResult HitResult;
	LineTrace(CameraStart, CameraEnd, Params, HitResult);

	// 트레이스 결과를 기반으로 최종 조준점 계산
	const bool bUseTraceResult = HitResult.bBlockingHit && (FVector::DistSquared(TraceStart, HitResult.Location) <= (MaxRange * MaxRange));
	const FVector AdjustedEnd = bUseTraceResult ? HitResult.Location : CameraEnd;

	// 플레이어 위치에서 조준점 방향으로 최종 종료점 계산
	FVector AdjustedAimDir = (AdjustedEnd - TraceStart).GetSafeNormal();
	if (AdjustedAimDir.IsZero())
	{
		AdjustedAimDir = CameraDirection;
	}

	OutTraceEnd = TraceStart + (AdjustedAimDir * MaxRange);
}

bool UAO_AbilityTask_WaitForInteractableTraceHit::ClipCameraRayToAbilityRange(
	FVector CameraLocation, 
	FVector CameraDirection, 
	FVector AbilityCenter, 
	float AbilityRange, 
	FVector& OutClippedPosition) const
{
	// 카메라 레이와 플레이어 중심 구체의 교점 계산
	FVector CameraToCenter = AbilityCenter - CameraLocation;
	float DistanceCameraToDot = FVector::DotProduct(CameraToCenter, CameraDirection);
	
	if (DistanceCameraToDot >= 0)
	{
		float DistanceSquared = CameraToCenter.SizeSquared() - (DistanceCameraToDot * DistanceCameraToDot);
		float RadiusSquared = (AbilityRange * AbilityRange);

		if (DistanceSquared <= RadiusSquared)
		{
			float DistanceDotToSphere = FMath::Sqrt(RadiusSquared - DistanceSquared);
			float DistanceCameraToSphere = DistanceCameraToDot + DistanceDotToSphere;

			OutClippedPosition = CameraLocation + (DistanceCameraToSphere * CameraDirection);
			return true;
		}
	}
	return false;
}

void UAO_AbilityTask_WaitForInteractableTraceHit::LineTrace(
	const FVector& Start, 
	const FVector& End, 
	const FCollisionQueryParams& Params, 
	FHitResult& OutHitResult) const
{
	// SphereCast로 감지
	TArray<FHitResult> HitResults;
	FCollisionShape Sphere = FCollisionShape::MakeSphere(TraceSphereRadius);
	GetWorld()->SweepMultiByChannel(
		HitResults,
		Start,
		End,
		FQuat::Identity,
		TraceChannel,
		Sphere,
		Params
	);
	
	if (HitResults.Num() > 0)
	{
		OutHitResult = HitResults[0];
	}
	else
	{
		OutHitResult = FHitResult();
		OutHitResult.TraceStart = Start;
		OutHitResult.TraceEnd = End;
	}
}

void UAO_AbilityTask_WaitForInteractableTraceHit::UpdateInteractionInfos(
	const FAO_InteractionQuery& InteractQuery, 
	const TArray<TScriptInterface<IAO_Interface_Interactable>>& Interactables)
{
	TArray<FAO_InteractionInfo> NewInteractionInfos;

	// 각 Interactable로부터 상호작용 정보 수집
	for (const TScriptInterface<IAO_Interface_Interactable>& Interactable : Interactables)
	{
		TArray<FAO_InteractionInfo> TempInteractionInfos;
		FAO_InteractionInfoBuilder InteractionInfoBuilder(Interactable, TempInteractionInfos);

		Interactable->GatherPostInteractionInfos(InteractQuery, InteractionInfoBuilder);

		// 상호작용 가능 여부 및 어빌리티 활성화 가능 여부 체크
		for (FAO_InteractionInfo& InteractionInfo : TempInteractionInfos)
		{
			if (InteractionInfo.AbilityToGrant)
			{
				// 해당 어빌리티가 이미 부여되어 있는지 확인
				FGameplayAbilitySpec* InteractionAbilitySpec = AbilitySystemComponent->FindAbilitySpecFromClass(InteractionInfo.AbilityToGrant);
				if (InteractionAbilitySpec)
				{
					// 상호작용 가능하고 어빌리티 활성화 가능한 경우만 추가
					if (Interactable->CanInteraction(InteractQuery) && 
						InteractionAbilitySpec->Ability->CanActivateAbility(InteractionAbilitySpec->Handle, AbilitySystemComponent->AbilityActorInfo.Get()))
					{
						NewInteractionInfos.Add(InteractionInfo);
					}
				}
			}
		}
	}

	// 이전 정보와 비교하여 변화 감지
	bool bInfosChanged = false;
	if (NewInteractionInfos.Num() == CurrentInteractionInfos.Num())
	{
		NewInteractionInfos.Sort();

		for (int InfoIndex = 0; InfoIndex < NewInteractionInfos.Num(); InfoIndex++)
		{
			const FAO_InteractionInfo& NewInfo = NewInteractionInfos[InfoIndex];
			const FAO_InteractionInfo& CurrentInfo = CurrentInteractionInfos[InfoIndex];

			if (NewInfo != CurrentInfo)
			{
				bInfosChanged = true;
				break;
			}
		}
	}
	else
	{
		bInfosChanged = true;
	}
	
	// 변화가 있으면 하이라이트 갱신 및 델리게이트 브로드캐스트
	if (bInfosChanged)
	{
		const bool bShouldOutlineEffect = InteractQuery.RequestingController.IsValid() && InteractQuery.RequestingController->IsLocalController();
		
		// 이전 하이라이트 제거
		if (bShouldOutlineEffect)
		{
			HighlightInteractables(CurrentInteractionInfos, false);
		}

		CurrentInteractionInfos = NewInteractionInfos;
		
		// 새 하이라이트 적용
		if (bShouldOutlineEffect)
		{
			HighlightInteractables(CurrentInteractionInfos, true);
		}
		
		// UI 업데이트를 위한 델리게이트 브로드캐스트
		InteractableChanged.Broadcast(CurrentInteractionInfos);
	}
}

void UAO_AbilityTask_WaitForInteractableTraceHit::HighlightInteractables(
	const TArray<FAO_InteractionInfo>& InteractionInfos, 
	bool bShouldHighlight)
{
	// 모든 Interactable의 메시 수집
	TArray<UMeshComponent*> MeshComponents;
	for (const FAO_InteractionInfo& InteractionInfo : InteractionInfos)
	{
		if (IAO_Interface_Interactable* Interactable = InteractionInfo.Interactable.GetInterface())
		{
			Interactable->GetMeshComponents(MeshComponents);
		}
	}

	// CustomDepth 설정으로 하이라이트 on/off
	// (포스트프로세스에서 CustomDepth=1인 오브젝트를 외곽선 표시)
	for (UMeshComponent* MeshComponent : MeshComponents)
	{
		MeshComponent->SetRenderCustomDepth(bShouldHighlight);
	}
}