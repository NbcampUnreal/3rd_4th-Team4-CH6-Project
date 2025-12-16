// HSJ : AO_InteractableComponent.cpp
#include "Interaction/Component/AO_InteractableComponent.h"

#include "NiagaraFunctionLibrary.h"
#include "Interaction/Base/GA_Interact_Base.h"
#include "Components/MeshComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Net/UnrealNetwork.h"
#include "Physics/AO_CollisionChannels.h"

UAO_InteractableComponent::UAO_InteractableComponent()
{
    PrimaryComponentTick.bCanEverTick = false;
    SetIsReplicatedByDefault(true);

    InteractionTitle = FText::FromString("Interact");
    InteractionContent = FText::FromString(": Press F");
}

void UAO_InteractableComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);
    
    DOREPLIFETIME(UAO_InteractableComponent, bInteractionEnabled);
}

FAO_InteractionInfo UAO_InteractableComponent::GetInteractionInfo(const FAO_InteractionQuery& InteractionQuery) const
{
    FAO_InteractionInfo Info;
    Info.Title = InteractionTitle;
    Info.Content = InteractionContent;
    Info.Duration = 0.0f;
    Info.AbilityToGrant = UGA_Interact_Base::StaticClass();
	Info.ActiveHoldMontage = ActiveHoldMontage;
	Info.ActiveMontage = ActiveMontage;
	Info.InteractionTransform = GetInteractionTransform();
	Info.WarpTargetName = WarpTargetName;
    return Info;
}

bool UAO_InteractableComponent::CanInteraction(const FAO_InteractionQuery& InteractionQuery) const
{
    // 상호작용 비활성화
    if (!bInteractionEnabled)
    {
        return false;
    }

    return true;
}

void UAO_InteractableComponent::GetMeshComponents(TArray<UMeshComponent*>& OutMeshComponents) const
{
	TObjectPtr<AActor> Owner = GetOwner();
	if (!Owner)
	{
		return;
	}

	// Owner의 모든 메시 컴포넌트 수집
	TArray<UMeshComponent*> AllMeshComponents;
	Owner->GetComponents<UMeshComponent>(AllMeshComponents, true);

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

// 상호작용 성공 시 호출
void UAO_InteractableComponent::NotifyInteractionSuccess(AActor* Interactor)
{
    TObjectPtr<AActor> Owner = GetOwner();
    if (!Owner || !Owner->HasAuthority())
    {
        return;
    }

	if (InteractionEffect.IsValid())
	{
		FVector Location = Owner->GetActorLocation();
		FRotator Rotation = Owner->GetActorRotation();
		MulticastPlayInteractionEffect(InteractionEffect, Location, Rotation);
	}
	
	OnInteractionSuccess.Broadcast(Interactor);
}

FTransform UAO_InteractableComponent::GetInteractionTransform() const
{
	TObjectPtr<AActor> Owner = GetOwner();
	if (!Owner) return FTransform::Identity;
    
	if (!InteractionSocketName.IsNone())
	{
		// Owner의 모든 메시 컴포넌트에서 Socket 찾기 (재귀)
		TArray<UMeshComponent*> MeshComponents;
		Owner->GetComponents<UMeshComponent>(MeshComponents, true);
        
		for (TObjectPtr<UMeshComponent> MeshComp : MeshComponents)
		{
			if (MeshComp && MeshComp->DoesSocketExist(InteractionSocketName))
			{
				return MeshComp->GetSocketTransform(InteractionSocketName);
			}
		}
	}
    
	return Owner->GetActorTransform();
}

void UAO_InteractableComponent::BeginPlay()
{
	Super::BeginPlay();
	
	SetupMeshCollisions();
}

void UAO_InteractableComponent::MulticastPlayInteractionEffect_Implementation(
	const FAO_InteractionEffectSettings& EffectSettings, FVector Location, FRotator Rotation)
{
	if (!EffectSettings.IsValid())
    {
        return;
    }

    FTransform SpawnTransform(Rotation, Location);

    // VFX Delay 처리
    if (EffectSettings.HasVFX())
    {
        if (EffectSettings.VFXSpawnDelay > 0.0f)
        {
            TObjectPtr<UWorld> World = GetWorld();
            if (World)
            {
                TWeakObjectPtr<UAO_InteractableComponent> WeakThis(this);
                
                World->GetTimerManager().SetTimer(
                    VFXSpawnTimerHandle,
                    FTimerDelegate::CreateWeakLambda(this, [WeakThis, EffectSettings, SpawnTransform]()
                    {
                        if (TObjectPtr<UAO_InteractableComponent> StrongThis = WeakThis.Get())
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
                TWeakObjectPtr<UAO_InteractableComponent> WeakThis(this);
                
                World->GetTimerManager().SetTimer(
                    SFXSpawnTimerHandle,
                    FTimerDelegate::CreateWeakLambda(this, [WeakThis, EffectSettings, SpawnTransform]()
                    {
                        if (TObjectPtr<UAO_InteractableComponent> StrongThis = WeakThis.Get())
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

void UAO_InteractableComponent::SetupMeshCollisions()
{
	TArray<UMeshComponent*> Meshes;
	GetMeshComponents(Meshes);
    
	for (TObjectPtr<UMeshComponent> Mesh : Meshes)
	{
		if (Mesh)
		{
			Mesh->SetCollisionResponseToChannel(AO_TraceChannel_Interaction, ECR_Block);
		}
	}
}

void UAO_InteractableComponent::SpawnVFXInternal(
    const FAO_InteractionEffectSettings& EffectSettings, 
    const FTransform& SpawnTransform)
{
    // Transform 계산
    FTransform FinalTransform = SpawnTransform;
    FinalTransform.AddToTranslation(FinalTransform.TransformVector(EffectSettings.VFXRelativeLocation));
    FinalTransform.SetRotation((FinalTransform.GetRotation() * EffectSettings.VFXRelativeRotation.Quaternion()));
    FinalTransform.SetScale3D(FinalTransform.GetScale3D() * EffectSettings.VFXScale);

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

    // MeshComponent 찾기
    TObjectPtr<UMeshComponent> OwnerMesh = GetOwner()->FindComponentByClass<UMeshComponent>();

    if (EffectSettings.NiagaraEffect)
    {
        if (EffectSettings.bAttachVFXToActor && OwnerMesh)
        {
            UNiagaraFunctionLibrary::SpawnSystemAttached(
                EffectSettings.NiagaraEffect,
                OwnerMesh,
                EffectSettings.VFXAttachSocketName,
                EffectSettings.VFXRelativeLocation,
                EffectSettings.VFXRelativeRotation,
                EffectSettings.VFXScale,
                AttachType,
                true,
                ENCPoolMethod::None,
                true
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
                true,
                true,
                ENCPoolMethod::None
            );
        }
    }
    else if (EffectSettings.CascadeEffect)
    {
        if (EffectSettings.bAttachVFXToActor && OwnerMesh)
        {
            UGameplayStatics::SpawnEmitterAttached(
                EffectSettings.CascadeEffect,
                OwnerMesh,
                EffectSettings.VFXAttachSocketName,
                EffectSettings.VFXRelativeLocation,
                EffectSettings.VFXRelativeRotation,
                EffectSettings.VFXScale,
                AttachType,
                true
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
                true
            );
        }
    }
}

void UAO_InteractableComponent::SpawnSFXInternal(
    const FAO_InteractionEffectSettings& EffectSettings, 
    const FTransform& SpawnTransform)
{
    if (!EffectSettings.Sound)
    {
        return;
    }

    TObjectPtr<UMeshComponent> OwnerMesh = GetOwner()->FindComponentByClass<UMeshComponent>();

    if (EffectSettings.bAttachSoundToActor && OwnerMesh)
    {
        UGameplayStatics::SpawnSoundAttached(
            EffectSettings.Sound,
            OwnerMesh,
            EffectSettings.SoundAttachSocketName,
            FVector::ZeroVector,
            FRotator::ZeroRotator,
            EAttachLocation::KeepRelativeOffset,
            false,
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