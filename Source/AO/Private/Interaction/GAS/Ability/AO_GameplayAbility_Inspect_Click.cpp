// HSJ : AO_GameplayAbility_Inspect_Click.cpp
#include "Interaction/GAS/Ability/AO_GameplayAbility_Inspect_Click.h"
#include "Interaction/Component/AO_InspectionComponent.h"
#include "Interaction/Interface/AO_Interface_Inspectable.h"
#include "Interaction/GAS/Tag/AO_InteractionGameplayTags.h"
#include "GameFramework/PlayerController.h"
#include "Engine/World.h"
#include "AO_Log.h"

UAO_GameplayAbility_Inspect_Click::UAO_GameplayAbility_Inspect_Click()
{
    InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;
    NetExecutionPolicy = EGameplayAbilityNetExecutionPolicy::LocalPredicted;

    // Inspection 중일 때만 활성화 가능
    ActivationRequiredTags.AddTag(AO_InteractionTags::Status_Action_Inspecting);
}

void UAO_GameplayAbility_Inspect_Click::ActivateAbility(
    const FGameplayAbilitySpecHandle Handle,
    const FGameplayAbilityActorInfo* ActorInfo,
    const FGameplayAbilityActivationInfo ActivationInfo,
    const FGameplayEventData* TriggerEventData)
{
    if (!CommitAbility(Handle, ActorInfo, ActivationInfo))
    {
        EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
        return;
    }

    AActor* AvatarActor = GetAvatarActorFromActorInfo();
    if (!AvatarActor)
    {
        EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
        return;
    }

	// InspectionComponent에서 현재 검사 중인 액터 확인
	UAO_InspectionComponent* InspectionComp = AvatarActor->FindComponentByClass<UAO_InspectionComponent>();
	if (!InspectionComp || !InspectionComp->GetInspectedActor())
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}

    APlayerController* PC = Cast<APlayerController>(AvatarActor->GetOwner());
    if (!PC)
    {
        EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
        return;
    }

    // 마우스 커서 위치에서 Trace
    FVector2D MousePosition;
    if (!PC->GetMousePosition(MousePosition.X, MousePosition.Y))
    {
        EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
        return;
    }

    FVector WorldLocation, WorldDirection;
    if (!PC->DeprojectScreenPositionToWorld(MousePosition.X, MousePosition.Y, WorldLocation, WorldDirection))
    {
        EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
        return;
    }

    FVector TraceStart = WorldLocation;
    FVector TraceEnd = WorldLocation + (WorldDirection * ClickTraceRange);

    FHitResult HitResult;
    FCollisionQueryParams QueryParams;
    QueryParams.AddIgnoredActor(AvatarActor);

    bool bHit = GetWorld()->LineTraceSingleByChannel(
        HitResult,
        TraceStart,
        TraceEnd,
        ECC_Visibility,
        QueryParams
    );

	if (bHit && HitResult.GetComponent() && HitResult.GetActor())
	{
		// InspectionComponent가 유효성 검사 담당
		if (InspectionComp->IsValidExternalClickTarget(HitResult.GetActor(), HitResult.GetComponent()))
		{
			InspectionComp->ServerProcessInspectionClick(HitResult.GetActor(), HitResult.GetComponent()->GetFName());
		}
	}

    EndAbility(Handle, ActorInfo, ActivationInfo, true, false);
}