// HSJ : AO_BaseInteractable.cpp
#include "Interaction/Base/AO_BaseInteractable.h"

#include "NiagaraFunctionLibrary.h"
#include "Components/StaticMeshComponent.h"
#include "Physics/AO_CollisionChannels.h"
#include "Interaction/Base/GA_Interact_Base.h"
#include "Kismet/GameplayStatics.h"
#include "Net/UnrealNetwork.h"
#include "Puzzle/Actor/AO_PuzzleReactionActor.h"

AAO_BaseInteractable::AAO_BaseInteractable(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	bReplicates = true;

	MeshComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("MeshComponent"));
	RootComponent = MeshComponent;

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

FAO_InteractionInfo AAO_BaseInteractable::GetInteractionInfo(const FAO_InteractionQuery& InteractionQuery) const
{
	FAO_InteractionInfo Info;
	Info.Title = InteractionTitle;
	Info.Content = InteractionContent;
	Info.Duration = InteractionDuration;
	Info.AbilityToGrant = UGA_Interact_Base::StaticClass();
	Info.ActiveHoldMontage  = ActiveHoldMontage;
	Info.ActiveMontage  = ActiveMontage;
	Info.DeactivateMontage = DeactivateMontage;
	Info.InteractionTransform = GetInteractionTransform();
	Info.WarpTargetName = WarpTargetName;
	Info.bWaitForAnimationNotify = bWaitForAnimationNotify;
	return Info;
}

void AAO_BaseInteractable::GetMeshComponents(TArray<UMeshComponent*>& OutMeshComponents) const
{
	// Actor의 모든 메시 컴포넌트 수집
	TArray<UMeshComponent*> AllMeshComponents;
	GetComponents<UMeshComponent>(AllMeshComponents, true);
    
	OutMeshComponents.Reserve(OutMeshComponents.Num() + AllMeshComponents.Num());
    
	for (TObjectPtr<UMeshComponent> MeshComp : AllMeshComponents)
	{
		if (!MeshComp) continue;
        
		// Static Mesh 체크
		if (TObjectPtr<UStaticMeshComponent> StaticMesh = Cast<UStaticMeshComponent>(MeshComp))
		{
			if (StaticMesh->GetStaticMesh())
			{
				OutMeshComponents.Add(MeshComp);
			}
		}
		// Skeletal Mesh 체크
		else if (TObjectPtr<USkeletalMeshComponent> SkeletalMesh = Cast<USkeletalMeshComponent>(MeshComp))
		{
			if (SkeletalMesh->GetSkeletalMeshAsset())
			{
				OutMeshComponents.Add(MeshComp);
			}
		}
	}
}

bool AAO_BaseInteractable::CanInteraction(const FAO_InteractionQuery& InteractionQuery) const
{
	return CanInteraction_BP(InteractionQuery);
}

bool AAO_BaseInteractable::CanInteraction_BP_Implementation(const FAO_InteractionQuery& InteractionQuery) const
{
	if (!Super::CanInteraction(InteractionQuery))
	{
		return false;
	}

	// 전역 비활성화 체크
	if (!bInteractionEnabled)
	{
		return false;
	}

	TObjectPtr<AActor> RequestingActor = InteractionQuery.RequestingAvatar.Get();
	if (IsPlayerDisabled(RequestingActor))
	{
		return false;
	}

	// 기본적으로 허용
	return true;
}

void AAO_BaseInteractable::OnInteractionSuccess(AActor* Interactor)
{
	Super::OnInteractionSuccess(Interactor);

	if (!HasAuthority())
	{
		return;
	}
	
	if (bIsToggleable)
	{
		bIsActivated = !bIsActivated;
		StartInteractionAnimation(bIsActivated);
		TriggerLinkedReactions(bIsActivated);

		// 이펙트 재생
		const FAO_InteractionEffectSettings& EffectToPlay = bIsActivated ? ActivateEffect : DeactivateEffect;
		if (EffectToPlay.IsValid())
		{
			MulticastPlayInteractionEffect(EffectToPlay, GetInteractionTransform());
		}
	}
	else
	{
		bIsActivated = true;
		StartInteractionAnimation(true);
		TriggerLinkedReactions(true);

		// 이펙트 재생
		if (ActivateEffect.IsValid())
		{
			MulticastPlayInteractionEffect(ActivateEffect, GetInteractionTransform());
		}
	}

	OnInteractionSuccess_BP(Interactor);
	MulticastNotifyInteractionSuccess(Interactor);
}

void AAO_BaseInteractable::MulticastNotifyInteractionSuccess_Implementation(AActor* Interactor)
{
	OnInteractionSuccessMulticast_BP(Interactor);
}

void AAO_BaseInteractable::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(ThisClass, bIsActivated);
	DOREPLIFETIME(ThisClass, bInteractionEnabled);
	DOREPLIFETIME(AAO_BaseInteractable, DisabledPlayers);
}

void AAO_BaseInteractable::OnInteractionSuccess_BP_Implementation(AActor* Interactor)
{
	// 기본 구현은 비어있음, 오버라이드 가능
}

void AAO_BaseInteractable::OnRep_IsActivated()
{
	StartInteractionAnimation(bIsActivated);
}

void AAO_BaseInteractable::TriggerLinkedReactions(bool bActivate)
{
	if (!HasAuthority())
	{
		return;
	}
    
	for (TObjectPtr<AAO_PuzzleReactionActor> ReactionActor : LinkedReactionActors)
	{
		if (!ReactionActor)
		{
			continue;
		}
        
		if (bActivate)
		{
			ReactionActor->ActivateReaction();
		}
		else
		{
			ReactionActor->DeactivateReaction();
		}
	}
}

void AAO_BaseInteractable::AddDisabledPlayer(AActor* Player)
{
	if (!HasAuthority()) 
	{
		return;
	}
	
	if (!Player) return;
	
	DisabledPlayers.AddUnique(Player);
}

void AAO_BaseInteractable::RemoveDisabledPlayer(AActor* Player)
{
	if (!HasAuthority()) 
	{
		return;
	}
	
	if (!Player) return;
	
	DisabledPlayers.Remove(Player);
}

bool AAO_BaseInteractable::IsPlayerDisabled(AActor* Player) const
{
	return Player && DisabledPlayers.Contains(Player);
}

void AAO_BaseInteractable::MulticastPlayInteractionEffect_Implementation(
	const FAO_InteractionEffectSettings& EffectSettings, FTransform SpawnTransform)
{
	if (!EffectSettings.IsValid())
    {
        return;
    }

    // VFX Delay 처리
    if (EffectSettings.HasVFX())
    {
        if (EffectSettings.VFXSpawnDelay > 0.0f)
        {
            TObjectPtr<UWorld> World = GetWorld();
            if (World)
            {
                TWeakObjectPtr<AAO_BaseInteractable> WeakThis(this);
                
                World->GetTimerManager().SetTimer(
                    VFXSpawnTimerHandle,
                    FTimerDelegate::CreateWeakLambda(this, [WeakThis, EffectSettings, SpawnTransform]()
                    {
                        if (TObjectPtr<AAO_BaseInteractable> StrongThis = WeakThis.Get())
                        {
                            StrongThis->SpawnVFXInternal(EffectSettings, SpawnTransform);
                        }
                    }),
                    EffectSettings.VFXSpawnDelay,
                    false
                );
            }
        }
        else
        {
            SpawnVFXInternal(EffectSettings, SpawnTransform);
        }
    }

    // SFX Delay 처리
    if (EffectSettings.HasSFX())
    {
        if (EffectSettings.SoundSpawnDelay > 0.0f)
        {
            TObjectPtr<UWorld> World = GetWorld();
            if (World)
            {
                TWeakObjectPtr<AAO_BaseInteractable> WeakThis(this);
                
                World->GetTimerManager().SetTimer(
                    SFXSpawnTimerHandle,
                    FTimerDelegate::CreateWeakLambda(this, [WeakThis, EffectSettings, SpawnTransform]()
                    {
                        if (TObjectPtr<AAO_BaseInteractable> StrongThis = WeakThis.Get())
                        {
                            StrongThis->SpawnSFXInternal(EffectSettings, SpawnTransform);
                        }
                    }),
                    EffectSettings.SoundSpawnDelay,
                    false
                );
            }
        }
        else
        {
            SpawnSFXInternal(EffectSettings, SpawnTransform);
        }
    }
}

FTransform AAO_BaseInteractable::GetInteractionTransform() const
{
	if (!MeshComponent || InteractionSocketName.IsNone())
	{
		return GetActorTransform();
	}

	// 루트 메시에서 먼저 Socket 찾기
	if (MeshComponent->DoesSocketExist(InteractionSocketName))
	{
		return MeshComponent->GetSocketTransform(InteractionSocketName);
	}

	// 자식 메시들에서 Socket 찾기
	TArray<USceneComponent*> ChildComponents;
	MeshComponent->GetChildrenComponents(true, ChildComponents);
    
	for (TObjectPtr<USceneComponent> Child : ChildComponents)
	{
		if (TObjectPtr<UMeshComponent> ChildMesh = Cast<UMeshComponent>(Child))
		{
			if (ChildMesh->DoesSocketExist(InteractionSocketName))
			{
				return ChildMesh->GetSocketTransform(InteractionSocketName);
			}
		}
	}

	return GetActorTransform();
}

void AAO_BaseInteractable::SpawnVFXInternal(
    const FAO_InteractionEffectSettings& EffectSettings, 
    const FTransform& SpawnTransform)
{
    // Transform 계산 (Relative 적용)
    FTransform FinalTransform = SpawnTransform;
    FinalTransform.AddToTranslation(FinalTransform.TransformVector(EffectSettings.VFXRelativeLocation));
    FinalTransform.SetRotation((FinalTransform.GetRotation() * EffectSettings.VFXRelativeRotation.Quaternion()));
    FinalTransform.SetScale3D(FinalTransform.GetScale3D() * EffectSettings.VFXScale);

    // EAttachmentRule을 EAttachLocation으로 변환
    EAttachLocation::Type AttachType = EAttachLocation::KeepRelativeOffset;
    switch (EffectSettings.VFXAttachLocationRule)
    {
    case EAttachmentRule::KeepRelative:
        AttachType = EAttachLocation::KeepRelativeOffset;
        break;
    case EAttachmentRule::KeepWorld:
        AttachType = EAttachLocation::KeepWorldPosition;
        break;
    case EAttachmentRule::SnapToTarget:
        AttachType = EAttachLocation::SnapToTarget;
        break;
    }

    // Niagara
    if (EffectSettings.NiagaraEffect)
    {
        if (EffectSettings.bAttachVFXToActor && MeshComponent)
        {
            UNiagaraFunctionLibrary::SpawnSystemAttached(
                EffectSettings.NiagaraEffect,
                MeshComponent,
                EffectSettings.VFXAttachSocketName,
                EffectSettings.VFXRelativeLocation,
                EffectSettings.VFXRelativeRotation,
                EffectSettings.VFXScale,
                AttachType,
                true,  // bAutoDestroy
                ENCPoolMethod::None,
                true   // bAutoActivate
            );
        }
        else
        {
            UNiagaraFunctionLibrary::SpawnSystemAtLocation(
                this,
                EffectSettings.NiagaraEffect,
                FinalTransform.GetLocation(),
                FinalTransform.Rotator(),
                FinalTransform.GetScale3D(),
                true,  // bAutoDestroy
                true,  // bAutoActivate
                ENCPoolMethod::None
            );
        }
    }
    // Cascade
    else if (EffectSettings.CascadeEffect)
    {
        if (EffectSettings.bAttachVFXToActor && MeshComponent)
        {
            UGameplayStatics::SpawnEmitterAttached(
                EffectSettings.CascadeEffect,
                MeshComponent,
                EffectSettings.VFXAttachSocketName,
                EffectSettings.VFXRelativeLocation,
                EffectSettings.VFXRelativeRotation,
                EffectSettings.VFXScale,
                AttachType,
                true  // bAutoDestroy
            );
        }
        else
        {
            UGameplayStatics::SpawnEmitterAtLocation(
                this,
                EffectSettings.CascadeEffect,
                FinalTransform.GetLocation(),
                FinalTransform.Rotator(),
                FinalTransform.GetScale3D(),
                true  // bAutoDestroy
            );
        }
    }
}

void AAO_BaseInteractable::SpawnSFXInternal(
    const FAO_InteractionEffectSettings& EffectSettings, 
    const FTransform& SpawnTransform)
{
    if (!EffectSettings.Sound)
    {
        return;
    }

    if (EffectSettings.bAttachSoundToActor && MeshComponent)
    {
        UGameplayStatics::SpawnSoundAttached(
            EffectSettings.Sound,
            MeshComponent,
            EffectSettings.SoundAttachSocketName,
            FVector::ZeroVector,
            FRotator::ZeroRotator,
            EAttachLocation::KeepRelativeOffset,
            false,  // bStopWhenAttachedToDestroyed
            EffectSettings.SoundVolumeMultiplier,
            EffectSettings.SoundPitchMultiplier
        );
    }
    else
    {
        UGameplayStatics::PlaySoundAtLocation(
            this,
            EffectSettings.Sound,
            SpawnTransform.GetLocation(),
            FRotator::ZeroRotator,
            EffectSettings.SoundVolumeMultiplier,
            EffectSettings.SoundPitchMultiplier
        );
    }
}
