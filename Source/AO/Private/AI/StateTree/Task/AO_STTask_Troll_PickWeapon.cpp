//KSJ : AO_STTask_Troll_PickWeapon

#include "AI/StateTree/Task/AO_STTask_Troll_PickWeapon.h"
#include "AI/Character/AO_Troll.h"
#include "AI/Controller/AO_TrollController.h"
#include "AI/Component/AO_WeaponHolderComp.h"
#include "AI/Item/AO_TrollWeapon.h"
#include "StateTreeExecutionContext.h"
#include "Navigation/PathFollowingComponent.h"
#include "Animation/AnimInstance.h"
#include "Components/SkeletalMeshComponent.h"

EStateTreeRunStatus FAO_STTask_Troll_PickWeapon::EnterState(FStateTreeExecutionContext& Context, const FStateTreeTransitionResult& Transition) const
{

	FAO_STTask_Troll_PickWeapon_InstanceData& InstanceData = Context.GetInstanceData<FAO_STTask_Troll_PickWeapon_InstanceData>(*this);
	InstanceData.bIsMoving = false;
	InstanceData.bIsPlayingPickupAnimation = false;
	InstanceData.TargetWeapon.Reset();

	AAO_TrollController* Controller = GetTrollController(Context);
	AAO_Troll* Troll = GetTroll(Context);

	if (!Controller || !Troll)
	{
		return EStateTreeRunStatus::Failed;
	}

	// 이미 무기를 들고 있으면 성공
	if (Troll->HasWeapon())
	{
		return EStateTreeRunStatus::Succeeded;
	}

	UAO_WeaponHolderComp* WeaponHolder = Troll->GetWeaponHolderComponent();
	if (!WeaponHolder)
	{
		return EStateTreeRunStatus::Failed;
	}

	// 가장 가까운 무기 찾기 (반경 기반으로 먼저 검색 - 시야 내 무기 목록이 갱신 안 될 수 있음)
	AAO_TrollWeapon* NearestWeapon = WeaponHolder->FindNearestWeaponInRadius(InstanceData.SearchRadius);
	if (!NearestWeapon)
	{
		NearestWeapon = WeaponHolder->FindNearestWeaponInSight();
	}

	if (!NearestWeapon)
	{
		return EStateTreeRunStatus::Failed;
	}
	

	InstanceData.TargetWeapon = NearestWeapon;

	// 무기 줍기 모드 설정
	Troll->SetPickingUpWeapon(true);

	// 무기로 이동
	if (MoveToWeapon(Controller, NearestWeapon, InstanceData.AcceptanceRadius))
	{
		InstanceData.bIsMoving = true;
	}
	else
	{
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
						return EStateTreeRunStatus::Succeeded;
					}
					else
					{
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
					return EStateTreeRunStatus::Running;
				}
			}
		}
		
		// 몽타주가 없거나 재생 실패 시 바로 줍기
		UAO_WeaponHolderComp* WeaponHolder = Troll->GetWeaponHolderComponent();
		if (WeaponHolder && WeaponHolder->PickupWeapon(TargetWeapon))
		{
			Troll->SetPickingUpWeapon(false);
			return EStateTreeRunStatus::Succeeded;
		}
		else
		{
			Troll->SetPickingUpWeapon(false);
			return EStateTreeRunStatus::Failed;
		}
	}

	// 아직 무기에 도달하지 못함 - 이동 상태 확인
	const EPathFollowingStatus::Type MoveStatus = Controller->GetMoveStatus();
	if (MoveStatus == EPathFollowingStatus::Idle)
	{
		// 이동이 완료되었지만 아직 줍지 못함 - 다시 이동 시도
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
	// 무기가 물리 시뮬레이션으로 움직일 수 있으므로 Actor를 직접 추적
	MoveRequest.SetGoalActor(Weapon);
	MoveRequest.SetAcceptanceRadius(AcceptRadius);
	MoveRequest.SetUsePathfinding(true);
	// 무기가 움직이면 경로를 업데이트하도록 설정
	MoveRequest.SetAllowPartialPath(true);

	const FPathFollowingRequestResult Result = Controller->MoveTo(MoveRequest);
	return Result.Code != EPathFollowingRequestResult::Failed;
}

