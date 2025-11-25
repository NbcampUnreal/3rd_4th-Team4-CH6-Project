// AO_TraversableComponent.cpp - KH

#include "Maps/Traversal/AO_TraversableComponent.h"

#include "AO_Log.h"
#include "Components/SplineComponent.h"

UAO_TraversableComponent::UAO_TraversableComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

void UAO_TraversableComponent::OnRegister()
{
	Super::OnRegister();

	ScanSplines();
}

void UAO_TraversableComponent::GetLedgeTransforms(FVector& HitLocation, FVector& ActorLocation,
	FTraversalCheckResult& TraversalResult)
{
	// Find the ledge closest to the actor
	USplineComponent* ClosestLedge = FindLedgeClosestToActor(ActorLocation);
	if (!ClosestLedge)
	{
		TraversalResult.bHasFrontLedge = false;
		return;
	}

	// Check if the ledge is wide enough
	if (ClosestLedge->GetSplineLength() < MinLedgeWidth)
	{
		TraversalResult.bHasFrontLedge = false;
		return;
	}

	// Get the closest point on the ledge to the actor
	// Local Space로 계산하는 이유: 스플라인의 거리 계산은 Local Space로 해야 되기 때문에 변환하는 과정이 필요함
	FVector ClosestLocation = ClosestLedge->FindLocationClosestToWorldLocation(HitLocation, ESplineCoordinateSpace::Local);
	float ClosestPoint = ClosestLedge->GetDistanceAlongSplineAtLocation(ClosestLocation, ESplineCoordinateSpace::Local);
	float ClampedPoint = FMath::Clamp(ClosestPoint, MinLedgeWidth / 2.0f, ClosestLedge->GetSplineLength() - MinLedgeWidth / 2.0f);
	FTransform ClosestTransform = ClosestLedge->GetTransformAtDistanceAlongSpline(ClampedPoint, ESplineCoordinateSpace::World);

	TraversalResult.bHasFrontLedge = true;
	TraversalResult.FrontLedgeLocation = ClosestTransform.GetLocation();
	TraversalResult.FrontLedgeNormal = ClosestTransform.GetRotation().GetUpVector();

	// Find the opposite ledge of the closest ledge using map
	USplineComponent* OppositeLedge = *OppositeLedges.Find(ClosestLedge);
	if (!OppositeLedge)
	{
		TraversalResult.bHasBackLedge = false;
		return;
	}

	// Get the closest point on the back ledge from the front ledge
	FTransform OppositeClosestTransform = OppositeLedge->FindTransformClosestToWorldLocation(TraversalResult.FrontLedgeLocation, ESplineCoordinateSpace::World);

	TraversalResult.bHasBackLedge = true;
	TraversalResult.BackLedgeLocation = OppositeClosestTransform.GetLocation();
	TraversalResult.BackLedgeNormal = OppositeClosestTransform.GetRotation().GetUpVector();
}

USplineComponent* UAO_TraversableComponent::FindLedgeClosestToActor(FVector& ActorLocation)
{
	if (Ledges.Num() == 0)
	{
		return nullptr;
	}	

	float ClosestDistance = 0.f;
	int32 ClosestIndex = 0;

	for (int32 i = 0; i < Ledges.Num(); ++i)
	{
		USplineComponent* Ledge = Ledges[i];

		// UpVector * 10을 해서 비교하는가? Ledge 위쪽의 실제 접근 위치를 기준으로 비교하기 위해서
		FVector ClosestLocation = Ledge->FindLocationClosestToWorldLocation(ActorLocation, ESplineCoordinateSpace::World);
		FVector ClosestUpVector = Ledge->FindUpVectorClosestToWorldLocation(ActorLocation, ESplineCoordinateSpace::World);
		ClosestUpVector *= 10.f;
		float Distance = FVector::Dist(ClosestLocation + ClosestUpVector, ActorLocation);

		if (i == 0 || Distance < ClosestDistance)
		{
			ClosestDistance = Distance;
			ClosestIndex = i;
		}
	}

	return Ledges[ClosestIndex];
}

void UAO_TraversableComponent::ScanSplines()
{
	Ledges.Empty();
	OppositeLedges.Empty();

	AActor* Owner = GetOwner();
	if (!Owner)
	{
		return;
	}

	TArray<USplineComponent*> SplineComponents;
	Owner->GetComponents(USplineComponent::StaticClass(), SplineComponents);

	for (USplineComponent* Spline : SplineComponents)
	{
		if (!Spline) continue;
		
		Ledges.Add(Spline);
	}

	for (int32 i = 0; i + 1 < Ledges.Num(); i += 2)
	{
		USplineComponent* A = Ledges[i];
		USplineComponent* B = Ledges[i + 1];

		if (!A || !B) continue;

		OppositeLedges.Add(A, B);
		OppositeLedges.Add(B, A);
	}
}