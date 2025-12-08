// AO_GameplayAbility_Traversal.cpp

#include "Character/GAS/Ability/AO_GameplayAbility_Traversal.h"

#include "AbilitySystemComponent.h"
#include "AnimationWarpingLibrary.h"
#include "ChooserFunctionLibrary.h"
#include "MotionWarpingComponent.h"
#include "Kismet/KismetMathLibrary.h"
#include "Components/CapsuleComponent.h"
#include "PoseSearch/PoseSearchLibrary.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Abilities/Tasks/AbilityTask_PlayMontageAndWait.h"

#include "Maps/Traversal/AO_TraversableComponent.h"
#include "Character/AO_PlayerCharacter.h"
#include "AO_Log.h"

UAO_GameplayAbility_Traversal::UAO_GameplayAbility_Traversal()
{
	InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;
	NetExecutionPolicy = EGameplayAbilityNetExecutionPolicy::LocalPredicted;

	const FGameplayTagContainer TraversalTag(FGameplayTag::RequestGameplayTag(FName("Ability.Movement.Traversal")));
	SetAssetTags(TraversalTag);

	ActivationOwnedTags.AddTag(FGameplayTag::RequestGameplayTag(FName("Status.Action.Traversal")));
}

void UAO_GameplayAbility_Traversal::OnAvatarSet(const FGameplayAbilityActorInfo* ActorInfo,
	const FGameplayAbilitySpec& Spec)
{
	Super::OnAvatarSet(ActorInfo, Spec);

	Owner = ActorInfo->AvatarActor.Get();
	checkf(Owner, TEXT("Failed to cast ActorInfo to AActor"));
	
	Character = Cast<ACharacter>(Owner);
	checkf(Character, TEXT("Failed to cast ActorInfo to ACharacter"));

	CharacterMovement = Character->GetCharacterMovement();
	checkf(CharacterMovement, TEXT("Failed to get CharacterMovementComponent"));

	CapsuleComponent = Cast<UCapsuleComponent>(Owner->GetComponentByClass(UCapsuleComponent::StaticClass()));
	checkf(CapsuleComponent, TEXT("Failed to cast Owner to UCapsuleComponent"));
}

bool UAO_GameplayAbility_Traversal::CanActivateAbility(const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo, const FGameplayTagContainer* SourceTags,
	const FGameplayTagContainer* TargetTags, FGameplayTagContainer* OptionalRelevantTags) const
{
	if (!Super::CanActivateAbility(Handle, ActorInfo, SourceTags, TargetTags, OptionalRelevantTags))
	{
		return false;
	}

	if (IsActive())
	{
		return false;
	}

	if (!const_cast<UAO_GameplayAbility_Traversal*>(this)->TryTraversal())
	{
		return false;
	}
	
	return true;
}

void UAO_GameplayAbility_Traversal::ActivateAbility(const FGameplayAbilitySpecHandle Handle,
                                                    const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo,
                                                    const FGameplayEventData* TriggerEventData)
{
	if (!CommitAbility(Handle, ActorInfo, ActivationInfo))
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}
	
	UpdateWarpTargets();

	if (CharacterMovement)
	{
		CharacterMovement->bIgnoreClientMovementErrorChecksAndCorrection = true;
		CharacterMovement->bServerAcceptClientAuthoritativePosition = true;
	}
	
	TObjectPtr<UAbilityTask_PlayMontageAndWait> MontageTask = UAbilityTask_PlayMontageAndWait::CreatePlayMontageAndWaitProxy(
		this,
		NAME_None,
		TraversalResult.ChosenMontage,
		TraversalResult.PlayRate,
		NAME_None,
		true,
		1.0f,
		TraversalResult.StartTime);
	
	MontageTask->OnCompleted.AddDynamic(this, &UAO_GameplayAbility_Traversal::OnMontageCompleted);
	MontageTask->OnInterrupted.AddDynamic(this, &UAO_GameplayAbility_Traversal::OnMontageCompleted);
	MontageTask->OnBlendOut.AddDynamic(this, &UAO_GameplayAbility_Traversal::OnMontageCompleted);
	MontageTask->OnCancelled.AddDynamic(this, &UAO_GameplayAbility_Traversal::OnMontageCompleted);
	
	MontageTask->ReadyForActivation();

	if (CharacterMovement)
	{
		CharacterMovement->SetMovementMode(MOVE_Flying);
	}
	
	if (ActorInfo->IsNetAuthority())
	{
		CapsuleComponent->IgnoreComponentWhenMoving(TraversalResult.HitComponent, true);
	}
}

void UAO_GameplayAbility_Traversal::EndAbility(const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo,
	bool bReplicateEndAbility, bool bWasCancelled)
{
	if (IsEndAbilityValid(Handle, ActorInfo))
	{
		if (ActorInfo->IsNetAuthority())
		{
			if (CapsuleComponent)
			{
				CapsuleComponent->IgnoreComponentWhenMoving(TraversalResult.HitComponent, false);
			}
		}

		if (CharacterMovement)
		{
			if (TraversalResult.ActionType == ETraversalActionType::Vault)
			{
				CharacterMovement->SetMovementMode(MOVE_Falling);
			}
			else
			{
				CharacterMovement->SetMovementMode(MOVE_Walking);
			}

			UKismetSystemLibrary::RetriggerableDelay(this, 0.2f, FLatentActionInfo());
			
			CharacterMovement->bIgnoreClientMovementErrorChecksAndCorrection = false;
			CharacterMovement->bServerAcceptClientAuthoritativePosition = false;
		}

		Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);
	}
}

bool UAO_GameplayAbility_Traversal::TryTraversal()
{
	// Detect Traversable Objects and acquire Information for Traversal 
	if (!DetectTraversal())
	{
		return false;
	}

	// Evaluate Traversal Conditions by Chooser Table
	TArray<TObjectPtr<UObject>> EvaluateObjects;
	if (!EvaluateTraversal(EvaluateObjects))
	{
		return false;
	}

	// Select the best Traversal Action by Motion Matching
	if (!SelectTraversal(EvaluateObjects))
	{
		return false;
	}
	
	return true;
}

bool UAO_GameplayAbility_Traversal::GetTraversalCheckInputs(FTraversalCheckInput& OutTraversalInput)
{
	if (!Character || !CharacterMovement)
	{
		return false;
	}
	
	if (CharacterMovement->MovementMode == MOVE_None
		|| CharacterMovement->MovementMode == MOVE_Walking
		|| CharacterMovement->MovementMode == MOVE_Custom)
	{
		FVector TraceDirection = UKismetMathLibrary::LessLess_VectorRotator(CharacterMovement->Velocity, Owner->GetActorRotation());
		OutTraversalInput.TraceForwardDistance = UKismetMathLibrary::MapRangeClamped(TraceDirection.X, 0.f, 500.f, 75.f, 350.f);
		OutTraversalInput.TraceForwardDirection = Owner->GetActorForwardVector();
		OutTraversalInput.TraceEndOffset.Z = 0.f;
		OutTraversalInput.TraceRadius = 30.f;
		OutTraversalInput.TraceHalfHeight = 60.f;
		return true;
	}
	
	if (CharacterMovement->MovementMode == MOVE_Falling
		|| CharacterMovement->MovementMode == MOVE_Flying)
	{
		OutTraversalInput.TraceForwardDirection = Owner->GetActorForwardVector();
		OutTraversalInput.TraceForwardDistance = 75.f;
		OutTraversalInput.TraceEndOffset.Z = 50.f;
		OutTraversalInput.TraceRadius = 30.f;
		OutTraversalInput.TraceHalfHeight = 86.f;
		return true;
	}

	return false;
}

bool UAO_GameplayAbility_Traversal::DetectTraversal()
{
	FTraversalCheckInput TraversalInput;
	if (!GetTraversalCheckInputs(TraversalInput))
	{
		AO_LOG(LogKH, Warning, TEXT("Failed to get inputs for traversal check"));
		return false;
	}

	const FVector ActorLocation = Owner->GetActorLocation();
	
	const float CapsuleRadius = CapsuleComponent->GetScaledCapsuleRadius();
	const float CapsuleHalfHeight = CapsuleComponent->GetScaledCapsuleHalfHeight();
	
	// Perform a trace in the actor's forward direction to find a traversable object
	FHitResult HitResult;
	const FVector TraceStart = ActorLocation + TraversalInput.TraceOriginOffset;
	const FVector TraceEnd = TraceStart + TraversalInput.TraceForwardDirection * TraversalInput.TraceForwardDistance + TraversalInput.TraceEndOffset;

	RunCapsuleTrace(
		TraceStart,
		TraceEnd,
		TraversalInput.TraceRadius,
		TraversalInput.TraceHalfHeight,
		HitResult,
		FColor::Black,
		FColor::Green);

	if (!HitResult.bBlockingHit)
	{
		if (DrawDebugLevel >= 1)
		{
			AO_LOG(LogKH, Display, TEXT("No blocking hit found"));
		}
		return false;
	}

	// Get the front and back ledge transforms from the traversable component
	TObjectPtr<UAO_TraversableComponent> TraversableComponent = Cast<UAO_TraversableComponent>(
		HitResult.GetActor()->GetComponentByClass(UAO_TraversableComponent::StaticClass()));
	if (!TraversableComponent)
	{
		if (DrawDebugLevel >= 1)
		{
			AO_LOG(LogKH, Display, TEXT("No TraversableComponent found on hit actor"));
		}
		return false;
	}

	TraversalResult.HitComponent = HitResult.GetComponent();
	TraversableComponent->GetLedgeTransforms(HitResult.ImpactPoint, ActorLocation, TraversalResult);

	if (!TraversalResult.bHasFrontLedge)
	{
		if (DrawDebugLevel >= 1)
		{
			AO_LOG(LogKH, Display, TEXT("No front ledge found"));
		}
		return false;
	}
	
	if (DrawDebugLevel >= 2)
	{
		if (TraversalResult.bHasFrontLedge)
		{
			DrawDebugSphere(GetWorld(), TraversalResult.FrontLedgeLocation, 10.f, 12,	FColor::Green, false, DrawDebugDuration, 0, 1.0f);
		}

		if (TraversalResult.bHasBackLedge)
		{
			DrawDebugSphere(GetWorld(), TraversalResult.BackLedgeLocation, 10.f, 12,	FColor::Cyan, false, DrawDebugDuration, 0, 1.0f);
		}
		else
		{
			AO_LOG(LogKH, Display, TEXT("No back ledge found"));
		}
	}
	
	// Perform a trace from the actors location up to the front ledge location to determine if there is room for the actor to move up to it
	FVector RoomCheckFrontLedgeLocation = TraversalResult.FrontLedgeLocation + TraversalResult.FrontLedgeNormal * (CapsuleRadius + 2.0f);
	RoomCheckFrontLedgeLocation.Z += CapsuleHalfHeight + 2.0f;

	FHitResult RoomCheckFrontHitResult;
	RunCapsuleTrace(
		ActorLocation,
		RoomCheckFrontLedgeLocation,
		CapsuleRadius,
		CapsuleHalfHeight,
		RoomCheckFrontHitResult,
		FColor::Red,
		FColor::Green);

	if (RoomCheckFrontHitResult.bBlockingHit)
	{
		if (DrawDebugLevel >= 1)
		{
			AO_LOG(LogKH, Display, TEXT("No room for actor to move up to front ledge"));
		}
		return false;
	}

	// Save the height of the obstacle
	TraversalResult.ObstacleHeight = abs((ActorLocation.Z - CapsuleHalfHeight) - TraversalResult.FrontLedgeLocation.Z);
	
	// Perform a trace across the top of the obstacle from the front ledge to the back ledge to see if theres room for the actor to move across it.
	FVector RoomCheckBackLedgeLocation = TraversalResult.BackLedgeLocation + TraversalResult.BackLedgeNormal * (CapsuleRadius + 2.0f);
	RoomCheckBackLedgeLocation.Z += CapsuleHalfHeight + 2.0f;

	FHitResult RoomCheckBackHitResult;
	const bool bRoomCheckBackHit = RunCapsuleTrace(
		RoomCheckFrontLedgeLocation,
		RoomCheckBackLedgeLocation,
		CapsuleRadius,
		CapsuleHalfHeight,
		RoomCheckBackHitResult,
		FColor::Red,
		FColor::Green);

	if (bRoomCheckBackHit)
	{
		TraversalResult.ObstacleDepth = (RoomCheckBackHitResult.ImpactPoint - TraversalResult.FrontLedgeLocation).Size2D();
		TraversalResult.bHasBackLedge = false;
	}
	else
	{
		TraversalResult.ObstacleDepth = (TraversalResult.FrontLedgeLocation - TraversalResult.BackLedgeLocation).Size2D();

		// Trace downward from the back ledge location to find the floor
		FVector BackFloorTrace = TraversalResult.BackLedgeLocation + TraversalResult.BackLedgeNormal * (CapsuleRadius + 2.0f);
		BackFloorTrace.Z -= 50.f;
		FHitResult BackFloorHitResult;
		RunCapsuleTrace(
			RoomCheckBackLedgeLocation,
			BackFloorTrace,
			CapsuleRadius,
			CapsuleHalfHeight,
			BackFloorHitResult,
			FColor::Red,
			FColor::Green);

		if (BackFloorHitResult.bBlockingHit)
		{
			TraversalResult.bHasBackFloor = true;
			TraversalResult.BackFloorLocation = BackFloorHitResult.ImpactPoint;
			TraversalResult.BackLedgeHeight = abs(BackFloorHitResult.ImpactPoint.Z - TraversalResult.BackLedgeLocation.Z);
		}
		else
		{
			TraversalResult.bHasBackFloor = false;
		}
	}

	// Debug Traversal Result
	if (DrawDebugLevel >= 1)
	{
		AO_LOG(LogKH, Display, TEXT("Has Front Ledge: %s"), TraversalResult.bHasFrontLedge ? TEXT("true") : TEXT("false"));
		AO_LOG(LogKH, Display, TEXT("Has Back Ledge: %s"), TraversalResult.bHasBackLedge ? TEXT("true") : TEXT("false"));
		AO_LOG(LogKH, Display, TEXT("Has Back Floor: %s"), TraversalResult.bHasBackFloor ? TEXT("true") : TEXT("false"));
		AO_LOG(LogKH, Display, TEXT("Obstacle Height: %f"), TraversalResult.ObstacleHeight);
		AO_LOG(LogKH, Display, TEXT("Obstacle Depth: %f"), TraversalResult.ObstacleDepth);
		AO_LOG(LogKH, Display, TEXT("Back Ledge Height: %f"), TraversalResult.BackLedgeHeight);
	}

	return true;
}

bool UAO_GameplayAbility_Traversal::EvaluateTraversal(TArray<TObjectPtr<UObject>>& EvaluateObjects)
{
	const TObjectPtr<AAO_PlayerCharacter> PlayerCharacter = Cast<AAO_PlayerCharacter>(Owner);
	checkf(PlayerCharacter, TEXT("Failed to cast Owner to AAO_PlayerCharacter"));

	checkf(AnimChooserTable, TEXT("TraversalChooserTable is not set"));
	
	// Evaluate a chooser to select all montages that match the conditions of the traversal check.
	const FInstancedStruct ResultInstances = UChooserFunctionLibrary::MakeEvaluateChooser(AnimChooserTable);
	FChooserEvaluationContext EvaluationContext;

	FInstancedStruct InputStruct;
	InputStruct.InitializeAs<FTraversalChooserInput>();
	FTraversalChooserInput& InputData = InputStruct.GetMutable<FTraversalChooserInput>();
	InputData.ActionType = TraversalResult.ActionType;
	InputData.bHasFrontLedge = TraversalResult.bHasFrontLedge;
	InputData.bHasBackLedge = TraversalResult.bHasBackLedge;
	InputData.bHasBackFloor = TraversalResult.bHasBackFloor;
	InputData.ObstacleHeight = TraversalResult.ObstacleHeight;
	InputData.ObstacleDepth = TraversalResult.ObstacleDepth;
	InputData.BackLedgeHeight = TraversalResult.BackLedgeHeight;
	InputData.MovementMode = CharacterMovement->MovementMode;
	InputData.Gait = PlayerCharacter->Gait;
	InputData.Speed2D = CharacterMovement->Velocity.Size2D();
	EvaluationContext.Params.Add(InputStruct);

	FInstancedStruct OutputStruct;
	OutputStruct.InitializeAs<FTraversalChooserOutput>();
	FTraversalChooserOutput& OutputData = OutputStruct.GetMutable<FTraversalChooserOutput>();
	EvaluationContext.Params.Add(OutputStruct);
	
	EvaluateObjects = UChooserFunctionLibrary::EvaluateObjectChooserBaseMulti(
		 EvaluationContext, ResultInstances, UAnimMontage::StaticClass());

	TraversalResult.ActionType = OutputData.ActionType;
	if (TraversalResult.ActionType == ETraversalActionType::None)
	{
		if (DrawDebugLevel >= 1)
		{
			AO_LOG(LogKH, Display, TEXT("No valid traversal action found"));
		}
		return false;
	}
	
	return true;
}

bool UAO_GameplayAbility_Traversal::SelectTraversal(const TArray<TObjectPtr<UObject>>& EvaluateObjects)
{
	// Perform a Motion Match on all the montages that were chosen by the chooser to find the best result.
	// This match will elect the best montage AND the best entry frame (start time) based on the distance to the ledge, and the current characters pose.
	FPoseSearchContinuingProperties ContinuingProperties;
	FPoseSearchFutureProperties FutureProperties;
	FPoseSearchBlueprintResult MotionMatchResult;
	UPoseSearchLibrary::MotionMatch(
		Character->GetMesh()->GetAnimInstance(),
		EvaluateObjects,
		FName(TEXT("PoseHistory")),
		ContinuingProperties,
		FutureProperties,
		MotionMatchResult);

	TObjectPtr<UAnimMontage> SelectedAnim = Cast<UAnimMontage>(MotionMatchResult.SelectedAnim);
	checkf(SelectedAnim, TEXT("Failed to cast SelectedAnim to UAnimMontage"));

	if (DrawDebugLevel >= 1)
	{
		AO_LOG(LogKH, Display, TEXT("Selected Anim: %s"), *SelectedAnim->GetName());
	}
	
	TraversalResult.ChosenMontage = SelectedAnim;
	TraversalResult.StartTime = MotionMatchResult.SelectedTime;
	TraversalResult.PlayRate = MotionMatchResult.WantedPlayRate;

	return true;
}

void UAO_GameplayAbility_Traversal::UpdateWarpTargets()
{
	TObjectPtr<UMotionWarpingComponent> MotionWarping = Cast<UMotionWarpingComponent>(
		Owner->GetComponentByClass(UMotionWarpingComponent::StaticClass()));
	if (!MotionWarping)
	{
		AO_LOG(LogKH, Warning, TEXT("Failed to get MotionWarpingComponent"));
		return;
	}

	MotionWarping->AddOrUpdateWarpTargetFromLocationAndRotation(
		FName(TEXT("FrontLedge")),
		TraversalResult.FrontLedgeLocation + FVector(0.f, 0.f, 0.5f),
		UKismetMathLibrary::MakeRotFromX(UKismetMathLibrary::NegateVector(TraversalResult.FrontLedgeNormal)));

	float DistanceFromFrontLedgeToBackLedge = 0.f;
	float DistanceFromFrontLedgeToBackFloor = 0.f;
	
	if (TraversalResult.ActionType == ETraversalActionType::Hurdle
		|| TraversalResult.ActionType == ETraversalActionType::Vault)
	{
		TArray<FMotionWarpingWindowData> OutWindows;
		UMotionWarpingUtilities::GetMotionWarpingWindowsForWarpTargetFromAnimation(TraversalResult.ChosenMontage, FName(TEXT("BackLedge")), OutWindows);

		if (OutWindows.Num() > 0)
		{
			UAnimationWarpingLibrary::GetCurveValueFromAnimation(
				TraversalResult.ChosenMontage,
				FName(TEXT("Distance_From_Ledge")),
				OutWindows[0].EndTime,
				DistanceFromFrontLedgeToBackLedge);

			MotionWarping->AddOrUpdateWarpTargetFromLocationAndRotation(
				FName(TEXT("BackLedge")),
				TraversalResult.BackLedgeLocation,
				FRotator::ZeroRotator);
		}
		else
		{
			MotionWarping->RemoveWarpTarget(FName(TEXT("BackLedge")));
		}
	}
	else
	{
		MotionWarping->RemoveWarpTarget(FName(TEXT("BackLedge")));
	}

	if (TraversalResult.ActionType == ETraversalActionType::Hurdle)
	{
		TArray<FMotionWarpingWindowData> OutWindows;
		UMotionWarpingUtilities::GetMotionWarpingWindowsForWarpTargetFromAnimation(TraversalResult.ChosenMontage, FName(TEXT("BackFloor")), OutWindows);

		if (OutWindows.Num() > 0)
		{
			UAnimationWarpingLibrary::GetCurveValueFromAnimation(
				TraversalResult.ChosenMontage,
				FName(TEXT("Distance_From_Ledge")),
				OutWindows[0].EndTime,
				DistanceFromFrontLedgeToBackFloor);

			FVector TargetLocation = TraversalResult.BackLedgeLocation
				+ TraversalResult.BackLedgeNormal * abs(DistanceFromFrontLedgeToBackLedge - DistanceFromFrontLedgeToBackFloor);
			TargetLocation.Z = TraversalResult.BackFloorLocation.Z;
			
			MotionWarping->AddOrUpdateWarpTargetFromLocationAndRotation(
				FName(TEXT("BackFloor")),
				TargetLocation,
				FRotator::ZeroRotator);
		}
		else
		{
			MotionWarping->RemoveWarpTarget(FName(TEXT("BackFloor")));
		}
	}
	else
	{
		MotionWarping->RemoveWarpTarget(FName(TEXT("BackFloor")));
	}
}

void UAO_GameplayAbility_Traversal::OnMontageCompleted()
{
	EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, false);
}

bool UAO_GameplayAbility_Traversal::RunCapsuleTrace(const FVector& StartLocation, const FVector& EndLocation, float Radius,
													float HalfHeight, FHitResult& OutHit, FColor DebugHitColor, FColor DebugTraceColor)
{
	const TObjectPtr<UWorld> World = GetWorld();
	checkf(World, TEXT("Failed to get World"));
	
	FCollisionShape TraceShape = FCollisionShape::MakeCapsule(Radius, HalfHeight);
	FCollisionQueryParams CollisionResponseParams(FName(TEXT("TraversalTrace")), false, Owner);
	const bool bHit = World->SweepSingleByChannel(
		OutHit,
		StartLocation,
		EndLocation,
		FQuat::Identity,
		ECC_Visibility,
		TraceShape,
		CollisionResponseParams
	);

	if (DrawDebugLevel >= 2)
	{
		const FQuat CapsuleRot = FQuat::Identity;

		if (bHit)
		{
			// 충돌 지점까지의 스윕 캡슐
			DrawDebugCapsule(World, OutHit.Location, HalfHeight, Radius, CapsuleRot, DebugHitColor, false, DrawDebugDuration);
			
			// 충돌이 시작된 곳에서 충돌 지점까지의 선
			DrawDebugLine(World, StartLocation, OutHit.Location, DebugHitColor, false, DrawDebugDuration);
		}
		else
		{
			// 전체 스윕 경로의 끝 지점 캡슐
			DrawDebugCapsule(World, EndLocation, HalfHeight, Radius, CapsuleRot, DebugTraceColor, false, DrawDebugDuration);

			// 전체 트레이스 선 (Start -> End)
			DrawDebugLine(World, StartLocation, EndLocation, DebugTraceColor, false, DrawDebugDuration);
		}
	}

	return bHit;
}
