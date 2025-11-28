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
	MeshComponent->SetCollisionObjectType(ECC_WorldDynamic);
	MeshComponent->SetCollisionResponseToAllChannels(ECR_Block);
	MeshComponent->SetCollisionResponseToChannel(AO_TraceChannel_Interaction, ECR_Block);
}

void AAO_PuzzleElement::BeginPlay()
{
	Super::BeginPlay();
}

void AAO_PuzzleElement::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	Super::EndPlay(EndPlayReason);
}

void AAO_PuzzleElement::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	
	DOREPLIFETIME(AAO_PuzzleElement, bIsActivated);
	DOREPLIFETIME(AAO_PuzzleElement, bInteractionEnabled);
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
		return false;

	// 상호작용 비활성화 체크
	if (!bInteractionEnabled)
		return false;

	// OneTime이고 이미 활성화되었으면 불가
	if (ElementType == EPuzzleElementType::OneTime && bIsActivated)
		return false;

	return true;
}

void AAO_PuzzleElement::GetMeshComponents(TArray<UMeshComponent*>& OutMeshComponents) const
{
	if (MeshComponent)
	{
		OutMeshComponents.Add(MeshComponent);
	}
}

void AAO_PuzzleElement::OnInteractionSuccess(AActor* Interactor)
{
	Super::OnInteractionSuccess(Interactor);

	if (!HasAuthority())
	{
		return;
	}

	if (!bHandleToggleInOnInteractionSuccess)
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
	}
}

void AAO_PuzzleElement::BroadcastPuzzleEvent(bool bActivate)
{
	if (!HasAuthority() || !LinkedChecker)
	{
		return;
	}

	if (bActivate)
	{
		// 활성화 태그 추가, 비활성화 태그 제거
		if (ActivatedEventTag.IsValid())
		{
			LinkedChecker->OnPuzzleEvent(ActivatedEventTag, GetInstigator());
		}
		if (DeactivatedEventTag.IsValid())
		{
			LinkedChecker->RemovePuzzleEvent(DeactivatedEventTag);
		}
	}
	else
	{
		// 비활성화 태그 추가, 활성화 태그 제거
		if (DeactivatedEventTag.IsValid())
		{
			LinkedChecker->OnPuzzleEvent(DeactivatedEventTag, GetInstigator());
		}
		if (ActivatedEventTag.IsValid())
		{
			LinkedChecker->RemovePuzzleEvent(ActivatedEventTag);
		}
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

void AAO_PuzzleElement::ResetToInitialState()
{
	if (!HasAuthority()) return;

	bIsActivated = false;
	bInteractionEnabled = true;
	CurrentHolder = nullptr;

	// 소모성도 리셋 (AAO_WorldInteractable의 bWasConsumed)
	bWasConsumed = false;

	OnRep_IsActivated(); // 클라이언트에 상태 변경 알림
}

void AAO_PuzzleElement::SetInteractionEnabled(bool bEnabled)
{
	if (!HasAuthority()) return;
	bInteractionEnabled = bEnabled;
}