// AO_STTask_Troll_PickWeapon.cpp

#include "AI/StateTree/Task/AO_STTask_Troll_PickWeapon.h"
#include "AI/Character/AO_Troll.h"
#include "AI/Controller/AO_TrollController.h"
#include "AI/Component/AO_WeaponHolderComp.h"
#include "AI/Item/AO_TrollWeapon.h"
#include "StateTreeExecutionContext.h"
#include "Navigation/PathFollowingComponent.h"
#include "Animation/AnimInstance.h"
#include "Components/SkeletalMeshComponent.h"
#include "AO_Log.h"

EStateTreeRunStatus FAO_STTask_Troll_PickWeapon::EnterState(FStateTreeExecutionContext& Context, const FStateTreeTransitionResult& Transition) const
{
	AO_LOG(LogKSJ, Log, TEXT("AO_STTask_Troll_PickWeapon: EnterState Called"));

	FAO_STTask_Troll_PickWeapon_InstanceData& InstanceData = Context.GetInstanceData<FAO_STTask_Troll_PickWeapon_InstanceData>(*this);
	InstanceData.bIsMoving = false;
	InstanceData.bIsPlayingPickupAnimation = false;
	InstanceData.TargetWeapon.Reset();

	AAO_TrollController* Controller = GetTrollController(Context);
	AAO_Troll* Troll = GetTroll(Context);

	if (!Controller || !Troll)
	{
		AO_LOG(LogKSJ, Error, TEXT("AO_STTask_Troll_PickWeapon: Controller or Troll is NULL"));
		return EStateTreeRunStatus::Failed;
	}

	// 이미 무기를 들고 있으면 성공
	if (Troll->HasWeapon())
	{
		AO_LOG(LogKSJ, Log, TEXT("AO_STTask_Troll_PickWeapon: Troll already has weapon"));
		return EStateTreeRunStatus::Succeeded;
	}

	UAO_WeaponHolderComp* WeaponHolder = Troll->GetWeaponHolderComponent();
	if (!WeaponHolder)
	{
		AO_LOG(LogKSJ, Error, TEXT("AO_STTask_Troll_PickWeapon: WeaponHolder is NULL"));
		return EStateTreeRunStatus::Failed;
	}

	// 가장 가까운 무기 찾기 (반경 기반으로 먼저 검색 - 시야 내 무기 목록이 갱신 안 될 수 있음)
	AAO_TrollWeapon* NearestWeapon = WeaponHolder->FindNearestWeaponInRadius(InstanceData.SearchRadius);
	if (!NearestWeapon)
	{
		AO_LOG(LogKSJ, Log, TEXT("AO_STTask_Troll_PickWeapon: No weapon in %.1f radius, checking sight..."), InstanceData.SearchRadius);
		NearestWeapon = WeaponHolder->FindNearestWeaponInSight();
	}

	if (!NearestWeapon)
	{
		AO_LOG(LogKSJ, Warning, TEXT("AO_STTask_Troll_PickWeapon: No weapon found nearby (Radius & Sight checked)"));
		return EStateTreeRunStatus::Failed;
	}
	
	AO_LOG(LogKSJ, Log, TEXT("AO_STTask_Troll_PickWeapon: Found weapon %s at distance %.1f"), 
		*NearestWeapon->GetName(), 
		FVector::Dist(Troll->GetActorLocation(), NearestWeapon->GetActorLocation()));

	InstanceData.TargetWeapon = NearestWeapon;

	// 무기 줍기 모드 설정
	Troll->SetPickingUpWeapon(true);

	// 무기로 이동
	if (MoveToWeapon(Controller, NearestWeapon, InstanceData.AcceptanceRadius))
	{
		InstanceData.bIsMoving = true;
		AO_LOG(LogKSJ, Log, TEXT("Troll moving to pickup weapon: %s"), *NearestWeapon->GetName());
	}
	else
	{
		AO_LOG(LogKSJ, Warning, TEXT("Troll failed to start move to weapon: %s"), *NearestWeapon->GetName());
	}

	return EStateTreeRunStatus::Running;
}

EStateTreeRunStatus FAO_STTask_Troll_PickWeapon::Tick(FStateTreeExecutionContext& Context, const float DeltaTime) const
{
	FAO_STTask_Troll_PickWeapon_InstanceData& InstanceData = Context.GetInstanceData<FAO_STTask_Troll_PickWeapon_InstanceData>(*this);

	AAO_TrollController* Controller = GetTrollController(Context);
	AAO_Troll* Troll = GetTroll(Context);

	if (!Controller || !Troll)
	{
		return EStateTreeRunStatus::Failed;
	}

	// 기절 상태면 중단
	if (Troll->IsStunned())
	{
		Troll->SetPickingUpWeapon(false);
		InstanceData.bIsPlayingPickupAnimation = false;
		return EStateTreeRunStatus::Failed;
	}

	// 이미 무기를 들고 있으면 성공
	if (Troll->HasWeapon())
	{
		Troll->SetPickingUpWeapon(false);
		InstanceData.bIsPlayingPickupAnimation = false;
		return EStateTreeRunStatus::Succeeded;
	}

	// 목표 무기가 유효한지 확인
	if (!InstanceData.TargetWeapon.IsValid())
	{
		Troll->SetPickingUpWeapon(false);
		InstanceData.bIsPlayingPickupAnimation = false;
		return EStateTreeRunStatus::Failed;
	}

	AAO_TrollWeapon* TargetWeapon = InstanceData.TargetWeapon.Get();

	// 다른 Troll이 먼저 주웠으면 실패
	if (TargetWeapon->IsPickedUp())
	{
		Troll->SetPickingUpWeapon(false);
		InstanceData.bIsPlayingPickupAnimation = false;
		return EStateTreeRunStatus::Failed;
	}

	// 무기 줍기 애니메이션 재생 중인지 확인
	if (InstanceData.bIsPlayingPickupAnimation)
	{
		USkeletalMeshComponent* Mesh = Troll->GetMesh();
		if (Mesh)
		{
			UAnimInstance* AnimInstance = Mesh->GetAnimInstance();
			if (AnimInstance)
			{
				// 몽타주가 재생 중이 아니면 줍기 완료
				if (!AnimInstance->IsAnyMontagePlaying())
				{
					// 무기 줍기 실행
					UAO_WeaponHolderComp* WeaponHolder = Troll->GetWeaponHolderComponent();
					if (WeaponHolder && WeaponHolder->PickupWeapon(TargetWeapon))
					{
						Troll->SetPickingUpWeapon(false);
						InstanceData.bIsPlayingPickupAnimation = false;
						AO_LOG(LogKSJ, Log, TEXT("Troll picked up weapon successfully after animation"));
						return EStateTreeRunStatus::Succeeded;
					}
					else
					{
						AO_LOG(LogKSJ, Warning, TEXT("Troll failed to pickup weapon after animation"));
						Troll->SetPickingUpWeapon(false);
						InstanceData.bIsPlayingPickupAnimation = false;
						return EStateTreeRunStatus::Failed;
					}
				}
			}
		}
		
		// 아직 애니메이션 재생 중
		return EStateTreeRunStatus::Running;
	}

	// 무기에 충분히 가까워졌는지 확인 (2D 거리 - 높이 차이 무시)
	const FVector TrollLoc = Troll->GetActorLocation();
	const FVector WeaponLoc = TargetWeapon->GetActorLocation();
	const float Dist2DSq = FVector::DistSquaredXY(TrollLoc, WeaponLoc);
	const float Dist2D = FMath::Sqrt(Dist2DSq);
	const float Dist3D = FVector::Dist(TrollLoc, WeaponLoc);
	
	if (Dist2DSq <= FMath::Square(InstanceData.PickupRadius))
	{
		AO_LOG(LogKSJ, Log, TEXT("Troll is close enough to weapon (Dist2D: %.1f, Dist3D: %.1f, PickupRadius: %.1f)"), 
			Dist2D, Dist3D, InstanceData.PickupRadius);
		
		// 몽타주가 설정되어 있으면 재생
		if (InstanceData.PickupMontage)
		{
			USkeletalMeshComponent* Mesh = Troll->GetMesh();
			if (Mesh)
			{
				UAnimInstance* AnimInstance = Mesh->GetAnimInstance();
				if (AnimInstance)
				{
					// 이동 중지
					Controller->StopMovement();
					InstanceData.bIsMoving = false;
					
					// 몽타주 재생
					AnimInstance->Montage_Play(InstanceData.PickupMontage, 1.0f);
					InstanceData.bIsPlayingPickupAnimation = true;
					AO_LOG(LogKSJ, Log, TEXT("Troll playing pickup montage: %s"), *InstanceData.PickupMontage->GetName());
					return EStateTreeRunStatus::Running;
				}
			}
		}
		
		// 몽타주가 없거나 재생 실패 시 바로 줍기
		UAO_WeaponHolderComp* WeaponHolder = Troll->GetWeaponHolderComponent();
		if (WeaponHolder && WeaponHolder->PickupWeapon(TargetWeapon))
		{
			Troll->SetPickingUpWeapon(false);
			AO_LOG(LogKSJ, Log, TEXT("Troll picked up weapon successfully (no montage)"));
			return EStateTreeRunStatus::Succeeded;
		}
		else
		{
			AO_LOG(LogKSJ, Warning, TEXT("Troll failed to pickup weapon - WeaponHolder: %s, TargetWeapon IsPickedUp: %s"), 
				WeaponHolder ? TEXT("Valid") : TEXT("Null"),
				TargetWeapon->IsPickedUp() ? TEXT("Yes") : TEXT("No"));
			Troll->SetPickingUpWeapon(false);
			return EStateTreeRunStatus::Failed;
		}
	}

	// 아직 무기에 도달하지 못함 - 이동 상태 확인
	const EPathFollowingStatus::Type MoveStatus = Controller->GetMoveStatus();
	if (MoveStatus == EPathFollowingStatus::Idle)
	{
		// 이동이 완료되었지만 아직 줍지 못함 - 다시 이동 시도
		AO_LOG(LogKSJ, Log, TEXT("Troll stopped but not close enough (Dist2D: %.1f, Dist3D: %.1f), retrying move"), 
			Dist2D, Dist3D);
		MoveToWeapon(Controller, TargetWeapon, InstanceData.AcceptanceRadius);
	}

	return EStateTreeRunStatus::Running;
}

void FAO_STTask_Troll_PickWeapon::ExitState(FStateTreeExecutionContext& Context, const FStateTreeTransitionResult& Transition) const
{
	FAO_STTask_Troll_PickWeapon_InstanceData& InstanceData = Context.GetInstanceData<FAO_STTask_Troll_PickWeapon_InstanceData>(*this);

	AAO_Troll* Troll = GetTroll(Context);
	if (Troll)
	{
		Troll->SetPickingUpWeapon(false);
		
		// 애니메이션 중지
		if (InstanceData.bIsPlayingPickupAnimation)
		{
			USkeletalMeshComponent* Mesh = Troll->GetMesh();
			if (Mesh)
			{
				UAnimInstance* AnimInstance = Mesh->GetAnimInstance();
				if (AnimInstance)
				{
					AnimInstance->Montage_Stop(0.2f);
				}
			}
			InstanceData.bIsPlayingPickupAnimation = false;
		}
	}

	if (InstanceData.bIsMoving)
	{
		AAO_TrollController* Controller = GetTrollController(Context);
		if (Controller)
		{
			Controller->StopMovement();
		}
		InstanceData.bIsMoving = false;
	}

	InstanceData.TargetWeapon.Reset();
}

AAO_TrollController* FAO_STTask_Troll_PickWeapon::GetTrollController(FStateTreeExecutionContext& Context) const
{
	if (AActor* Owner = Cast<AActor>(Context.GetOwner()))
	{
		if (AAO_TrollController* Controller = Cast<AAO_TrollController>(Owner))
		{
			return Controller;
		}
		if (APawn* Pawn = Cast<APawn>(Owner))
		{
			return Cast<AAO_TrollController>(Pawn->GetController());
		}
	}
	return nullptr;
}

AAO_Troll* FAO_STTask_Troll_PickWeapon::GetTroll(FStateTreeExecutionContext& Context) const
{
	AAO_TrollController* Controller = GetTrollController(Context);
	if (Controller)
	{
		return Controller->GetTroll();
	}
	return nullptr;
}

bool FAO_STTask_Troll_PickWeapon::MoveToWeapon(AAO_TrollController* Controller, AAO_TrollWeapon* Weapon, float AcceptRadius) const
{
	if (!Controller || !Weapon)
	{
		return false;
	}

	FAIMoveRequest MoveRequest;
	MoveRequest.SetGoalLocation(Weapon->GetActorLocation());
	MoveRequest.SetAcceptanceRadius(AcceptRadius);
	MoveRequest.SetUsePathfinding(true);

	const FPathFollowingRequestResult Result = Controller->MoveTo(MoveRequest);
	return Result.Code != EPathFollowingRequestResult::Failed;
}

