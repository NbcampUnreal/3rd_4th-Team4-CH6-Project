// HSJ : AO_AbilityTask_WaitForInvalidInteraction.cpp
#include "Interaction/GAS/Task/AO_AbilityTask_WaitForInvalidInteraction.h"
#include "AO_Log.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/KismetMathLibrary.h"

UAO_AbilityTask_WaitForInvalidInteraction::UAO_AbilityTask_WaitForInvalidInteraction(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
}

UAO_AbilityTask_WaitForInvalidInteraction* UAO_AbilityTask_WaitForInvalidInteraction::WaitForInvalidInteraction(
	UGameplayAbility* OwningAbility, 
	float AcceptanceAngle, 
	float AcceptanceDistance)
{
	UAO_AbilityTask_WaitForInvalidInteraction* Task = NewAbilityTask<UAO_AbilityTask_WaitForInvalidInteraction>(OwningAbility);
	Task->AcceptanceAngle = AcceptanceAngle;
	Task->AcceptanceDistance = AcceptanceDistance;
	return Task;
}

void UAO_AbilityTask_WaitForInvalidInteraction::Activate()
{
	Super::Activate();

	SetWaitingOnAvatar();

	// 시작 시점의 방향과 위치 저장
	CachedCharacterForward2D = GetAvatarActor() ? GetAvatarActor()->GetActorForwardVector().GetSafeNormal2D() : FVector::ZeroVector;
	CachedCharacterLocation = GetAvatarActor() ? GetAvatarActor()->GetActorLocation() : FVector::ZeroVector;

	// 0.1초마다 체크 시작
	GetWorld()->GetTimerManager().SetTimer(CheckTimerHandle, this, &ThisClass::PerformCheck, 0.1f, true);
}

void UAO_AbilityTask_WaitForInvalidInteraction::OnDestroy(bool bInOwnerFinished)
{
	// 타이머 정리
	GetWorld()->GetTimerManager().ClearTimer(CheckTimerHandle);
	
	Super::OnDestroy(bInOwnerFinished);
}

void UAO_AbilityTask_WaitForInvalidInteraction::PerformCheck()
{
	ACharacter* Character = Cast<ACharacter>(Ability->GetCurrentActorInfo()->AvatarActor.Get());
	UCharacterMovementComponent* CharacterMovement = Character ? Character->GetCharacterMovement() : nullptr;

	if (!Character || !CharacterMovement)
	{
		AO_LOG(LogHSJ, Error, TEXT("Character or CharacterMovement is null"));
		OnInvalidInteraction.Broadcast();
		EndTask();
		return;
	}

	// 현재 상태 계산
	float CurrentAngle = CalculateAngle2D();
	float CurrentDistXY = FVector::Dist2D(CachedCharacterLocation, Character->GetActorLocation());
	float CurrentDistZ = FMath::Abs(CachedCharacterLocation.Z - Character->GetActorLocation().Z);
	
	// Z축은 앉기 높이만큼 여유 허용
	float MaxDistZ = AcceptanceDistance + CharacterMovement->GetCrouchedHalfHeight();

	// 유효성 검증
	bool bValidAngle2D = CurrentAngle <= AcceptanceAngle;
	bool bValidDistanceXY = FVector::DistSquared2D(CachedCharacterLocation, Character->GetActorLocation()) <= (AcceptanceDistance * AcceptanceDistance);
	bool bValidDistanceZ = CurrentDistZ <= MaxDistZ;

	// 검증 통과 시 계속 체크
	if (bValidAngle2D && bValidDistanceXY && bValidDistanceZ)
	{
		return;
	}

	
	AO_LOG(LogHSJ, Warning, TEXT("Interaction invalidated - Angle: %.1f/%.1f, DistXY: %.1f/%.1f, DistZ: %.1f/%.1f"), 
		CurrentAngle, AcceptanceAngle, 
		CurrentDistXY, AcceptanceDistance,
		CurrentDistZ, MaxDistZ);
	
	// 검증 실패 시 취소
	OnInvalidInteraction.Broadcast();
	EndTask();
}

float UAO_AbilityTask_WaitForInvalidInteraction::CalculateAngle2D() const
{
	AActor* AvatarActor = Ability->GetCurrentActorInfo()->AvatarActor.Get();
	APlayerController* PlayerController = Ability->GetCurrentActorInfo()->PlayerController.Get();
	
	if (!AvatarActor || !PlayerController)
	{
		return 0.f;
	}

	// 시작 방향과 현재 방향의 내적으로 각도 계산
	FVector CharacterForward2D = AvatarActor->GetActorForwardVector().GetSafeNormal2D();
	return UKismetMathLibrary::DegAcos(CachedCharacterForward2D.Dot(CharacterForward2D));
}