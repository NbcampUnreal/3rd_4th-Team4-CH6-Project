// HSJ : AO_PressurePlate.cpp
#include "Puzzle/Element/AO_PressurePlate.h"
#include "Components/BoxComponent.h"
#include "GameFramework/Pawn.h"

AAO_PressurePlate::AAO_PressurePlate(const FObjectInitializer& ObjectInitializer)
    : Super(ObjectInitializer)
{
    // 압력판은 OneTime 타입 기반 (한번 활성화 후 유지되는 게 아니라 Overlap으로 제어)
    ElementType = EPuzzleElementType::OneTime;
	AnimationSpeed = 5.0f;

    OverlapTrigger = CreateDefaultSubobject<UBoxComponent>(TEXT("OverlapTrigger"));
    OverlapTrigger->SetupAttachment(RootComponent);
    OverlapTrigger->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
    OverlapTrigger->SetCollisionResponseToAllChannels(ECR_Ignore);
    OverlapTrigger->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);
    OverlapTrigger->SetGenerateOverlapEvents(true);
}

void AAO_PressurePlate::BeginPlay()
{
    Super::BeginPlay();

	if (MeshComponent)
	{
		InitialMeshLocation = MeshComponent->GetRelativeLocation();
	}

    if (OverlapTrigger)
    {
        OverlapTrigger->OnComponentBeginOverlap.AddDynamic(this, &AAO_PressurePlate::OnOverlapBegin);
        OverlapTrigger->OnComponentEndOverlap.AddDynamic(this, &AAO_PressurePlate::OnOverlapEnd);
    }
}

void AAO_PressurePlate::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	UWorld* World = GetWorld();
	if (World)
	{
		World->GetTimerManager().ClearTimer(PlateAnimationTimerHandle);
	}
	Super::EndPlay(EndPlayReason);
}

bool AAO_PressurePlate::CanInteraction(const FAO_InteractionQuery& InteractionQuery) const
{
    // F키 상호작용 비활성화
    return false;
}

void AAO_PressurePlate::ResetToInitialState()
{
	Super::ResetToInitialState();
    
	UWorld* World = GetWorld();
	if (World)
	{
		World->GetTimerManager().ClearTimer(PlateAnimationTimerHandle);
	}
    
	if (MeshComponent)
	{
		MeshComponent->SetRelativeLocation(InitialMeshLocation);
	}
}

void AAO_PressurePlate::OnOverlapBegin(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, 
    UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
    if (!HasAuthority()) return;
    if (!bInteractionEnabled) return;

    APawn* Pawn = Cast<APawn>(OtherActor);
    if (!Pawn) return;

    if (OverlappingActors.Contains(OtherActor)) return;

    OverlappingActors.Add(OtherActor);

    // 첫 번째 액터가 들어오면 활성화
    if (OverlappingActors.Num() == 1)
    {
        bIsActivated = true;
        BroadcastPuzzleEvent(true);
        OnRep_IsActivated();

    	// 눌리는 애니메이션
    	TargetMeshLocation = InitialMeshLocation - FVector(0, 0, PressDepth);
    	StartPlateAnimation();
    }
}

void AAO_PressurePlate::OnOverlapEnd(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, 
    UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
    if (!HasAuthority()) return;

    OverlappingActors.Remove(OtherActor);

    // 모든 액터가 나가면 비활성화
    if (OverlappingActors.Num() == 0)
    {
        bIsActivated = false;
        BroadcastPuzzleEvent(false);
        OnRep_IsActivated();

    	// 원위치 애니메이션
    	TargetMeshLocation = InitialMeshLocation;
    	StartPlateAnimation();
    }
}

void AAO_PressurePlate::StartPlateAnimation()
{
	if (!MeshComponent) return;
    
	UWorld* World = GetWorld();
	checkf(World, TEXT("World is null in StartPlateAnimation"));
    
	TWeakObjectPtr<AAO_PressurePlate> WeakThis(this);
    
	World->GetTimerManager().SetTimer(
		PlateAnimationTimerHandle,
		FTimerDelegate::CreateWeakLambda(this, [WeakThis]()
		{
			if (AAO_PressurePlate* StrongThis = WeakThis.Get())
			{
				StrongThis->UpdatePlateAnimation();
			}
		}),
		0.016f,
		true
	);
}

void AAO_PressurePlate::UpdatePlateAnimation()
{
	if (!MeshComponent) return;
    
	FVector CurrentLocation = MeshComponent->GetRelativeLocation();
	FVector NewLocation = FMath::VInterpTo(CurrentLocation, TargetMeshLocation, 0.016f, AnimationSpeed);
    
	MeshComponent->SetRelativeLocation(NewLocation);
    
	if (FVector::Dist(NewLocation, TargetMeshLocation) < 0.1f)
	{
		MeshComponent->SetRelativeLocation(TargetMeshLocation);
		UWorld* World = GetWorld();
		if (World)
		{
			World->GetTimerManager().ClearTimer(PlateAnimationTimerHandle);
		}
	}
}

void AAO_PressurePlate::OnRep_IsActivated()
{
	Super::OnRep_IsActivated();
    
	// 클라이언트에서도 애니메이션 재생
	if (!HasAuthority() && MeshComponent)
	{
		TargetMeshLocation = bIsActivated ? (InitialMeshLocation - FVector(0, 0, PressDepth)) : InitialMeshLocation;
		StartPlateAnimation();
	}
}