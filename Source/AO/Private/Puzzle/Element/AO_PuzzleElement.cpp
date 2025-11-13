// HSJ : AO_PuzzleElement.cpp
#include "Puzzle/Element/AO_PuzzleElement.h"
#include "AO_Log.h"
#include "Net/UnrealNetwork.h"
#include "Interaction/Data/AO_InteractionDataAsset.h"
#include "Puzzle/Checker/AO_PuzzleConditionChecker.h"
#include "Components/StaticMeshComponent.h"
#include "Physics/AO_CollisionChannels.h"

AAO_PuzzleElement::AAO_PuzzleElement(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	bReplicates = true;

	// StaticMeshComponent 생성
	MeshComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("MeshComponent"));
	RootComponent = MeshComponent;

	// 상호작용 가능하도록 콜리전 설정
	MeshComponent->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	MeshComponent->SetCollisionObjectType(AO_TraceChannel_Interaction);
	MeshComponent->SetCollisionResponseToAllChannels(ECR_Ignore);
	MeshComponent->SetCollisionResponseToChannel(AO_TraceChannel_Interaction, ECR_Block);
}

void AAO_PuzzleElement::BeginPlay()
{
	Super::BeginPlay();

	// 런타임에 메시/머티리얼 적용
	if (MeshComponent)
	{
		// 메시 설정
		if (PuzzleMesh)
		{
			MeshComponent->SetStaticMesh(PuzzleMesh);
		}

		// 머티리얼 설정
		for (int32 i = 0; i < PuzzleMaterials.Num(); ++i)
		{
			if (PuzzleMaterials[i])
			{
				MeshComponent->SetMaterial(i, PuzzleMaterials[i]);
			}
		}
	}
}

#if WITH_EDITOR
void AAO_PuzzleElement::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
	Super::PostEditChangeProperty(PropertyChangedEvent);

	const FName PropertyName = PropertyChangedEvent.GetPropertyName();

	// 에디터에서 메시 변경 시 즉시 적용
	if (PropertyName == GET_MEMBER_NAME_CHECKED(AAO_PuzzleElement, PuzzleMesh))
	{
		if (MeshComponent && PuzzleMesh)
		{
			MeshComponent->SetStaticMesh(PuzzleMesh);
		}
	}

	// 에디터에서 머티리얼 변경 시 즉시 적용
	if (PropertyName == GET_MEMBER_NAME_CHECKED(AAO_PuzzleElement, PuzzleMaterials))
	{
		if (MeshComponent)
		{
			for (int32 i = 0; i < PuzzleMaterials.Num(); ++i)
			{
				if (PuzzleMaterials[i])
				{
					MeshComponent->SetMaterial(i, PuzzleMaterials[i]);
				}
			}
		}
	}
}
#endif

void AAO_PuzzleElement::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	
	DOREPLIFETIME(AAO_PuzzleElement, bIsActivated);
}

FAO_InteractionInfo AAO_PuzzleElement::GetInteractionInfo(const FAO_InteractionQuery& InteractionQuery) const
{
	// DataAsset 사용 시 우선
	if (bUseInteractionDataAsset && InteractionDataAsset)
	{
		return InteractionDataAsset->InteractionInfo;
	}
	
	// 직접 설정한 정보 반환
	return PuzzleInteractionInfo;
}

bool AAO_PuzzleElement::CanInteraction(const FAO_InteractionQuery& InteractionQuery) const
{
	if (!Super::CanInteraction(InteractionQuery))
	{
		return false;
	}

	// OneTime이고 이미 활성화되었으면 불가
	if (ElementType == EPuzzleElementType::OneTime && bIsActivated)
	{
		return false;
	}

	return true;
}

void AAO_PuzzleElement::GetMeshComponents(TArray<UMeshComponent*>& OutMeshComponents) const
{
	if (MeshComponent)
	{
		OutMeshComponents.Add(MeshComponent);
	}
}

void AAO_PuzzleElement::OnInteractActiveStarted(AActor* Interactor)
{
	Super::OnInteractActiveStarted(Interactor);

	if (!HasAuthority())
	{
		return;
	}

	// HoldContinuous만 홀딩 시작 시 즉시 활성화
	if (ElementType == EPuzzleElementType::HoldContinuous)
	{
		CurrentHolder = Interactor;
		BroadcastPuzzleEvent(true);
	}
}

void AAO_PuzzleElement::OnInteractActiveEnded(AActor* Interactor)
{
	Super::OnInteractActiveEnded(Interactor);

	if (!HasAuthority())
	{
		return;
	}

	// HoldContinuous만 홀딩 종료 시 즉시 비활성화
	if (ElementType == EPuzzleElementType::HoldContinuous && CurrentHolder == Interactor)
	{
		CurrentHolder = nullptr;
		BroadcastPuzzleEvent(false);
	}
}

void AAO_PuzzleElement::OnInteractionSuccess(AActor* Interactor)
{
	Super::OnInteractionSuccess(Interactor);

	if (!HasAuthority())
	{
		return;
	}

	// ElementType에 따른 상태 변경
	switch (ElementType)
	{
	case EPuzzleElementType::OneTime:
		// 한 번만 활성화
		if (!bIsActivated)
		{
			bIsActivated = true;
			BroadcastPuzzleEvent(true);
		}
		break;

	case EPuzzleElementType::Toggle:
		// 즉시 토글
		bIsActivated = !bIsActivated;
		BroadcastPuzzleEvent(bIsActivated);
		break;

	case EPuzzleElementType::HoldToggle:
		// 홀딩 완료 후 토글
		bIsActivated = !bIsActivated;
		BroadcastPuzzleEvent(bIsActivated);
		break;

	case EPuzzleElementType::HoldContinuous:
		// OnInteractActiveStarted/Ended에서 처리함
		break;
	}
}

void AAO_PuzzleElement::BroadcastPuzzleEvent(bool bActivate)
{
	if (!HasAuthority() || !LinkedChecker)
	{
		return;
	}

	// 활성화/비활성화에 따른 태그 선택
	FGameplayTag EventTag = bActivate ? ActivatedEventTag : DeactivatedEventTag;
	
	if (EventTag.IsValid())
	{
		// LinkedChecker에 이벤트 전송
		LinkedChecker->OnPuzzleEvent(EventTag, GetInstigator());
		AO_LOG_NET(LogHSJ, Log, TEXT("Broadcast event: %s"), *EventTag.ToString());
	}
	else
	{
		AO_LOG_NET(LogHSJ, Warning, TEXT("EventTag is invalid"));
	}
}

void AAO_PuzzleElement::SetActivationState(bool bNewState)
{
	if (!HasAuthority())
	{
		return;
	}

	// 상태가 실제로 변경될 때만 이벤트 전송
	if (bIsActivated != bNewState)
	{
		bIsActivated = bNewState;
		BroadcastPuzzleEvent(bIsActivated);
	}
}

void AAO_PuzzleElement::OnRep_IsActivated()
{
	// 클라이언트에서 상태 변경 시 블루프린트 이벤트 호출
	OnElementStateChanged(bIsActivated);
}