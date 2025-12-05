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

	InteractableMeshComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("InteractableMeshComponent"));
	InteractableMeshComponent->SetupAttachment(MeshComponent);
	InteractableMeshComponent->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	InteractableMeshComponent->SetCollisionObjectType(ECC_WorldDynamic);
	InteractableMeshComponent->SetCollisionResponseToAllChannels(ECR_Block);
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
	FAO_InteractionInfo Info = bUseInteractionDataAsset && InteractionDataAsset 
		? InteractionDataAsset->InteractionInfo 
		: PuzzleInteractionInfo;
    
	Info.InteractionTransform = GetInteractionTransform();
	Info.WarpTargetName = WarpTargetName;
    
	return Info;
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
	if (!MeshComponent)
	{
		return;
	}

	// 루트 메시가 실제 메시를 가지고 있으면 추가
	if (UStaticMeshComponent* StaticMesh = Cast<UStaticMeshComponent>(MeshComponent))
	{
		if (StaticMesh->GetStaticMesh())
		{
			OutMeshComponents.Add(MeshComponent);
		}
	}

	// 자식 메시 컴포넌트들도 모두 수집
	TArray<USceneComponent*> ChildComponents;
	MeshComponent->GetChildrenComponents(true, ChildComponents);
    
	for (USceneComponent* Child : ChildComponents)
	{
		if (UMeshComponent* ChildMesh = Cast<UMeshComponent>(Child))
		{
			if (UStaticMeshComponent* StaticMesh = Cast<UStaticMeshComponent>(ChildMesh))
			{
				if (StaticMesh->GetStaticMesh())
				{
					OutMeshComponents.Add(ChildMesh);
				}
			}
		}
	}
}

void AAO_PuzzleElement::OnInteractionSuccess(AActor* Interactor)
{
	Super::OnInteractionSuccess(Interactor);

	if (!HasAuthority())
	{
		return;
	}

	// 사운드 재생
	MulticastPlayInteractionSound();

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
			StartInteractionAnimation(true);
		}
		break;

	case EPuzzleElementType::Toggle:
		// 즉시 토글
		bIsActivated = !bIsActivated;
		BroadcastPuzzleEvent(bIsActivated);
		StartInteractionAnimation(bIsActivated);
		break;

	case EPuzzleElementType::HoldToggle:
		// 홀딩 완료 후 토글
		bIsActivated = !bIsActivated;
		BroadcastPuzzleEvent(bIsActivated);
		StartInteractionAnimation(bIsActivated);
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
	// 클라이언트 애니메이션
	if (!HasAuthority())
	{
		StartInteractionAnimation(bIsActivated);
	}
}

FTransform AAO_PuzzleElement::GetInteractionTransform() const
{
	if (!MeshComponent || InteractionSocketName.IsNone())
	{
		return GetActorTransform();
	}

	// 루트 메시에서 Socket 찾기
	if (MeshComponent->DoesSocketExist(InteractionSocketName))
	{
		return MeshComponent->GetSocketTransform(InteractionSocketName);
	}

	// 자식 메시들에서 Socket 찾기
	TArray<USceneComponent*> ChildComponents;
	MeshComponent->GetChildrenComponents(true, ChildComponents);
    
	for (USceneComponent* Child : ChildComponents)
	{
		if (UMeshComponent* ChildMesh = Cast<UMeshComponent>(Child))
		{
			if (ChildMesh->DoesSocketExist(InteractionSocketName))
			{
				return ChildMesh->GetSocketTransform(InteractionSocketName);
			}
		}
	}

	return GetActorTransform();
}

void AAO_PuzzleElement::ResetToInitialState()
{
	checkf(HasAuthority(), TEXT("ResetToInitialState called on client"));

	bIsActivated = false;
	bInteractionEnabled = true;
	CurrentHolder = nullptr;

	// 소모성도 리셋 (AAO_WorldInteractable의 bWasConsumed)
	bWasConsumed = false;

	UWorld* World = GetWorld();
	if (World)
	{
		World->GetTimerManager().ClearTimer(TransformAnimationTimerHandle);
	}
    
	if (InteractableMeshComponent && bUseTransformAnimation)
	{
		InteractableMeshComponent->SetRelativeLocation(InitialInteractableLocation);
		InteractableMeshComponent->SetRelativeRotation(InitialInteractableRotation);
	}

	OnRep_IsActivated(); // 클라이언트에 상태 변경 알림
}

void AAO_PuzzleElement::SetInteractionEnabled(bool bEnabled)
{
	checkf(HasAuthority(), TEXT("SetInteractionEnabled called on client"));
	bInteractionEnabled = bEnabled;
}