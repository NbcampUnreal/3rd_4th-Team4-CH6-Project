//KSJ : AO_Stalker

#include "AI/Character/AO_Stalker.h"
#include "AI/Component/AO_CeilingMoveComponent.h"
#include "AI/Animation/AO_Stalker_AnimInstance.h"
#include "AI/Controller/AO_StalkerController.h"
#include "Components/CapsuleComponent.h"
#include "GameFramework/CharacterMovementComponent.h"

AAO_Stalker::AAO_Stalker()
{
	// 캡슐 콜리전 크기 설정 (네발로 기어다니는 자세, 낮고 넓음)
	GetCapsuleComponent()->SetCapsuleHalfHeight(60.f);
	GetCapsuleComponent()->SetCapsuleRadius(80.f);

	CeilingMoveComp = CreateDefaultSubobject<UAO_CeilingMoveComponent>(TEXT("CeilingMoveComp"));

	// NavAgentProps 설정 (천장 모드에서 바닥 NavMesh를 찾기 위해)
	UCharacterMovementComponent* MoveComp = GetCharacterMovement();
	if (MoveComp)
	{
		// 기본 NavAgentProps 설정
		MoveComp->NavAgentProps.bCanCrouch = false;
		MoveComp->NavAgentProps.bCanJump = false;
		// AgentHeight와 AgentRadius는 캡슐 크기에 맞게 자동 설정됨
	}

	// 속도 설정
	// KSJ: 요구사항 반영
	// - 배회: 300
	// - 추격(지상 스토킹/기습 포함): 500
	// - 천장 이동(배회에서만): 600 (ChaseSpeed * 1.2f 로 계산)
	RoamSpeed = 300.f;
	ChaseSpeed = 500.f;
	
	DefaultMovementSpeed = RoamSpeed;
	AlertMovementSpeed = ChaseSpeed;

	AIControllerClass = AAO_StalkerController::StaticClass();
}

void AAO_Stalker::BeginPlay()
{
	Super::BeginPlay();
}

void AAO_Stalker::SetCeilingMode(bool bEnable)
{
	if (CeilingMoveComp)
	{
		CeilingMoveComp->SetCeilingMode(bEnable);
		
		// 천장 이동 시 속도 증가
		if (bEnable)
		{
			GetCharacterMovement()->MaxWalkSpeed = ChaseSpeed * 1.2f;
		}
		else
		{
			GetCharacterMovement()->MaxWalkSpeed = ChaseSpeed;
		}
	}
}

void AAO_Stalker::PlayCeilingTransitionMontage(bool bToCeiling)
{
	if (bIsTransitioningCeiling)
	{
		return;
	}

	// 1) Stalker BP에 세팅된 몽타주 우선 사용
	UAnimMontage* MontageToPlay = bToCeiling ? JumpToCeilingMontage : JumpToFloorMontage;

	// 2) 폴백: CeilingMoveComponent에 세팅된 몽타주 사용 (유저가 여기만 세팅했을 수도 있음)
	if (!MontageToPlay && CeilingMoveComp)
	{
		MontageToPlay = bToCeiling ? CeilingMoveComp->GetJumpUpMontage() : CeilingMoveComp->GetJumpDownMontage();
	}
	
	// 몽타주 없으면 즉시 전환
	if (!MontageToPlay)
	{
		SetCeilingMode(bToCeiling);
		return;
	}

	// 이미 원하는 모드라면 무시 (상태 일치)
	bool bCurrentMode = false;
	if (CeilingMoveComp)
	{
		bCurrentMode = CeilingMoveComp->IsInCeilingMode();
	}
	
	if (bCurrentMode == bToCeiling)
	{
		return;
	}

	// 몽타주 재생
	if (USkeletalMeshComponent* MeshComp = GetMesh())
	{
		if (UAnimInstance* AnimInst = MeshComp->GetAnimInstance())
		{
			const float Duration = AnimInst->Montage_Play(MontageToPlay);
			if (Duration > 0.f)
			{
				bIsTransitioningCeiling = true;
				bPendingCeilingMode = bToCeiling;

				// 몽타주 종료 델리게이트 바인딩
				FOnMontageEnded EndedDelegate;
				EndedDelegate.BindUObject(this, &AAO_Stalker::OnCeilingTransitionMontageEnded);
				AnimInst->Montage_SetEndDelegate(EndedDelegate, MontageToPlay);

				// 이동 정지 (애니메이션 재생 중 움직임 방지)
				if (AController* Ctrl = GetController())
				{
					Ctrl->StopMovement();
				}
			}
			else
			{
				// 재생 실패 시 즉시 전환
				SetCeilingMode(bToCeiling);
			}
		}
	}
}

void AAO_Stalker::OnCeilingTransitionMontageEnded(UAnimMontage* Montage, bool bInterrupted)
{
	bIsTransitioningCeiling = false;
	
	// KSJ: AnimNotify가 몽타주 중간에 SetCeilingMode()를 호출하므로,
	// 여기서는 호출하지 않음. (중복 방지)
	// 만약 AnimNotify가 없거나 실패한 경우를 대비해, bInterrupted가 false일 때만 확인
	if (!bInterrupted)
	{
		// AnimNotify가 정상적으로 호출되었다면 이미 모드가 전환되어 있을 것임
		// 하지만 안전장치로, 아직 전환되지 않았다면 여기서 전환
		if (CeilingMoveComp)
		{
			bool bCurrentMode = CeilingMoveComp->IsInCeilingMode();
			if (bCurrentMode != bPendingCeilingMode)
			{
				// AnimNotify가 호출되지 않았거나 실패한 경우 (폴백)
				SetCeilingMode(bPendingCeilingMode);
			}
		}
	}
}

void AAO_Stalker::SetRetreatMode(bool bRetreat)
{
	bIsRetreating = bRetreat;
	
	// Retreat 모드 활성화 시 천장 모드 해제 (바닥으로 내려옴)
	if (bRetreat && CeilingMoveComp && CeilingMoveComp->IsInCeilingMode())
	{
		// 가능한 경우 점프 다운 몽타주를 재생한 뒤 바닥 모드로 전환
		PlayCeilingTransitionMontage(false);
	}
}

void AAO_Stalker::HandleStunBegin()
{
	Super::HandleStunBegin();

	// 전환 중이었다면 취소
	bIsTransitioningCeiling = false;
	if (USkeletalMeshComponent* MeshComp = GetMesh())
	{
		if (UAnimInstance* AnimInst = MeshComp->GetAnimInstance())
		{
			AnimInst->Montage_Stop(0.2f);
		}
	}

	// 기절 시 천장에서 떨어짐
	SetCeilingMode(false);

	// 서버에서 Multicast로 기절 애니메이션 재생 (모든 클라이언트에서 보이도록)
	if (HasAuthority())
	{
		if (UAO_Stalker_AnimInstance* StalkerAnimInstance = Cast<UAO_Stalker_AnimInstance>(GetMesh()->GetAnimInstance()))
		{
			UAnimMontage* StunMontage = StalkerAnimInstance->GetStunMontage();
			if (StunMontage)
			{
				Multicast_PlayStunMontage(StunMontage, StalkerAnimInstance->GetStunMontagePlayRate());
			}
		}
	}
}

void AAO_Stalker::HandleStunEnd()
{
	Super::HandleStunEnd();

	// 서버에서 Multicast로 기절 애니메이션 중지
	if (HasAuthority())
	{
		Multicast_StopStunMontage(0.25f);
	}
}

