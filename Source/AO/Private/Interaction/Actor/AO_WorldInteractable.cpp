// HSJ :  AO_WorldInteractable.cpp
#include "Interaction/Actor/AO_WorldInteractable.h"
#include "AbilitySystemBlueprintLibrary.h"
#include "AbilitySystemComponent.h"
#include "AO_Log.h"
#include "Interaction/GAS/Tag/AO_InteractionGameplayTags.h"
#include "Kismet/GameplayStatics.h"
#include "Net/UnrealNetwork.h"
#include "Physics/AO_CollisionChannels.h"

AAO_WorldInteractable::AAO_WorldInteractable(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	bReplicates = true;
}

void AAO_WorldInteractable::BeginPlay()
{
	Super::BeginPlay();

	// 초기 Transform 저장
	UStaticMeshComponent* InteractableMesh = GetInteractableMesh();
	if (InteractableMesh && bUseTransformAnimation)
	{
		InitialInteractableLocation = InteractableMesh->GetRelativeLocation();
		InitialInteractableRotation = InteractableMesh->GetRelativeRotation();
	}
}

void AAO_WorldInteractable::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(ThisClass, bWasConsumed);
}

void AAO_WorldInteractable::OnInteractActiveStarted(AActor* Interactor)
{
	if (!IsValid(Interactor))
	{
		return;
	}
	
	// 서버: 홀딩 중인 플레이어 추적
	if (HasAuthority())
	{
		CachedInteractors.Add(Interactor);
	}

	// 블루프린트 이벤트 호출
	K2_OnInteractActiveStarted(Interactor);
}

void AAO_WorldInteractable::OnInteractActiveEnded(AActor* Interactor)
{
	if (!IsValid(Interactor))
	{
		return;
	}
	
	// 서버: 홀딩 종료한 플레이어 제거
	if (HasAuthority())
	{
		CachedInteractors.RemoveSingleSwap(Interactor);
	}

	// 블루프린트 이벤트 호출
	K2_OnInteractActiveEnded(Interactor);
}

void AAO_WorldInteractable::OnInteractionSuccess(AActor* Interactor)
{
	if (!IsValid(Interactor))
	{
		return;
	}
	
	if (HasAuthority())
	{
		// 소모성 상호작용 처리
		if (bShouldConsume)
		{
			bWasConsumed = true;

			// 다른 홀딩 중인 플레이어들의 상호작용 취소
			// (먼저 성공한 플레이어가 독점)
			TArray<TWeakObjectPtr<AActor>> TargetInteractors = MoveTemp(CachedInteractors);

			for (TWeakObjectPtr<AActor>& TargetInteractor : TargetInteractors)
			{
				AActor* TargetActor = TargetInteractor.Get();
				if (!TargetActor || Interactor == TargetActor)
				{
					continue;
				}

				// 다른 플레이어의 상호작용 어빌리티 강제 취소
				if (UAbilitySystemComponent* ASC = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(TargetActor))
				{
					FGameplayTagContainer CancelAbilitiesTag;
					CancelAbilitiesTag.AddTag(AO_InteractionTags::Status_Action_AbilityInteract);
					ASC->CancelAbilities(&CancelAbilitiesTag);
				}
			}
		}
		else
		{
			// 비소모성: 홀딩 목록만 정리
			CachedInteractors.Empty();
		}
	}
	
	// 블루프린트 이벤트 호출
	K2_OnInteractionSuccess(Interactor);
}

bool AAO_WorldInteractable::CanInteraction(const FAO_InteractionQuery& InteractionQuery) const
{
	// 소모성이면 아직 소모 안 됐을 때만 가능
	// 비소모성이면 항상 가능
	return bShouldConsume ? !bWasConsumed : true;
}

void AAO_WorldInteractable::OnRep_WasConsumed()
{
	// 소모 상태 복제 완료 (클라이언트에서 UI 업데이트 등에 사용 가능)
}

void AAO_WorldInteractable::StartInteractionAnimation(bool bActivate)
{
    UStaticMeshComponent* InteractableMesh = GetInteractableMesh();

    if (!InteractableMesh || !bUseTransformAnimation)
    {
        return;
    }

    // 목표 위치/회전 계산
    if (bActivate)
    {
        TargetLocation = bUseLocation ? TargetRelativeLocation : InitialInteractableLocation;
        TargetRotation = bUseRotation ? TargetRelativeRotation : InitialInteractableRotation;
    }
    else
    {
        TargetLocation = InitialInteractableLocation;
        TargetRotation = InitialInteractableRotation;
    }

	UWorld* World = GetWorld();
	checkf(World, TEXT("World is null in StartInteractionAnimation"));

    // 타이머 시작
    TWeakObjectPtr<AAO_WorldInteractable> WeakThis(this);
	World->GetTimerManager().SetTimer(
		TransformAnimationTimerHandle,
		FTimerDelegate::CreateWeakLambda(this, [WeakThis]()
		{
			if (AAO_WorldInteractable* StrongThis = WeakThis.Get())
			{
				StrongThis->UpdateTransformAnimation();
			}
		}),
		0.016f,
		true
	);
}

void AAO_WorldInteractable::UpdateTransformAnimation()
{
    UStaticMeshComponent* InteractableMesh = GetInteractableMesh();
    if (!InteractableMesh)
    {
        return;
    }

    FVector CurrentLocation = InteractableMesh->GetRelativeLocation();
    FRotator CurrentRotation = InteractableMesh->GetRelativeRotation();

    FVector NewLocation = FMath::VInterpTo(CurrentLocation, TargetLocation, 0.016f, AnimationSpeed);
    FRotator NewRotation = FMath::RInterpTo(CurrentRotation, TargetRotation, 0.016f, AnimationSpeed);

    InteractableMesh->SetRelativeLocation(NewLocation);
    InteractableMesh->SetRelativeRotation(NewRotation);

    bool bReachedTarget = FVector::Dist(NewLocation, TargetLocation) < 0.1f &&
                          IsRotatorNearlyEqual(NewRotation, TargetRotation, 0.5f);

    if (bReachedTarget)
    {
        InteractableMesh->SetRelativeLocation(TargetLocation);
        InteractableMesh->SetRelativeRotation(TargetRotation);
    	UWorld* World = GetWorld();
    	if (World)
    	{
    		World->GetTimerManager().ClearTimer(TransformAnimationTimerHandle);
    	}
    }
}

void AAO_WorldInteractable::MulticastPlayInteractionSound_Implementation()
{
	if (InteractionSound)
	{
		UGameplayStatics::PlaySoundAtLocation(
			this,
			InteractionSound,
			GetActorLocation(),
			FRotator::ZeroRotator,
			SoundVolumeMultiplier,
			SoundPitchMultiplier
		);
	}
}

bool AAO_WorldInteractable::IsRotatorNearlyEqual(const FRotator& A, const FRotator& B, float Tolerance) const
{
    return FMath::Abs(FRotator::NormalizeAxis(A.Pitch - B.Pitch)) <= Tolerance &&
           FMath::Abs(FRotator::NormalizeAxis(A.Yaw - B.Yaw)) <= Tolerance &&
           FMath::Abs(FRotator::NormalizeAxis(A.Roll - B.Roll)) <= Tolerance;
}

void AAO_WorldInteractable::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	UWorld* World = GetWorld();
	if (World)
	{
		World->GetTimerManager().ClearTimer(TransformAnimationTimerHandle);
	}
    Super::EndPlay(EndPlayReason);
}