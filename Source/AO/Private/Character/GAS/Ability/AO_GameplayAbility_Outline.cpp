// AO_GameplayAbility_Outline.cpp

#include "Character/GAS/Ability/AO_GameplayAbility_Outline.h"

#include "EngineUtils.h"
#include "GameFramework/Character.h"
#include "Components/PostProcessComponent.h"
#include "Abilities/Tasks/AbilityTask_WaitDelay.h"

UAO_GameplayAbility_Outline::UAO_GameplayAbility_Outline()
	: OutlineMaterial(nullptr)
	, OutlineMaterial_Clip(nullptr)
	, DynamicMaterial(nullptr)
	, DynamicMaterial_Clip(nullptr)
	, Opacity(0.2f)
	, Color(0.05f, 0.f, 0.02f)
	, OutlineOpacity(0.5f)
	, OutlineColor(1.f, 0.f, 0.06f)
	, AbilityDuration(3.f)
	, OutlineActorClass(nullptr)
{
	InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;
	NetExecutionPolicy = EGameplayAbilityNetExecutionPolicy::LocalPredicted;
}

void UAO_GameplayAbility_Outline::ActivateAbility(const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo,
	const FGameplayEventData* TriggerEventData)
{
	if (!CommitAbility(Handle, ActorInfo, ActivationInfo))
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}

	ACharacter* Character = Cast<ACharacter>(ActorInfo->AvatarActor.Get());
	if (!Character)
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}

	if (ActorInfo->IsLocallyControlled())
	{
		if (OutlineMaterial)
		{
			DynamicMaterial = UMaterialInstanceDynamic::Create(OutlineMaterial, this);
			SetMaterialParams(DynamicMaterial);
			if (UPostProcessComponent* PPC = Character->FindComponentByClass<UPostProcessComponent>())
			{
				FWeightedBlendable Blendable(1.0f, DynamicMaterial);
				PPC->Settings.WeightedBlendables.Array.Add(Blendable);
			}
		}

		if (OutlineMaterial_Clip)
		{
			DynamicMaterial_Clip = UMaterialInstanceDynamic::Create(OutlineMaterial_Clip, this);
			SetMaterialParams(DynamicMaterial_Clip);
			if (UPostProcessComponent* PPC = Character->FindComponentByClass<UPostProcessComponent>())
			{
				FWeightedBlendable Blendable(1.0f, DynamicMaterial_Clip);
				PPC->Settings.WeightedBlendables.Array.Add(Blendable);
			}
		}
	
		if (OutlineActorClass)
		{
			if (UWorld* World = GetWorld())
			{
				for (TActorIterator<AActor> It(World, OutlineActorClass); It; ++It)
				{
					AActor* Target = *It;
					if (Target && Target != Character)
					{
						SetCustomDepth(Target, true);
						OutlinedActors.Add(Target);
					}
				}
			}
		}
	}

	if (UAbilityTask_WaitDelay* WaitTask = UAbilityTask_WaitDelay::WaitDelay(this, AbilityDuration))
	{
		WaitTask->OnFinish.AddDynamic(this, &ThisClass::OnDurationFinished);
		WaitTask->ReadyForActivation();
	}
}

void UAO_GameplayAbility_Outline::EndAbility(const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo,
	bool bReplicateEndAbility, bool bWasCancelled)
{
	
	ACharacter* Character = Cast<ACharacter>(ActorInfo->AvatarActor.Get());
	if (Character && ActorInfo->IsLocallyControlled())
	{
		if (UPostProcessComponent* PPC = Character->FindComponentByClass<UPostProcessComponent>())
		{
			PPC->Settings.WeightedBlendables.Array.RemoveAll([this](const FWeightedBlendable& WB)
			{
				return WB.Object == DynamicMaterial || WB.Object == DynamicMaterial_Clip;
			});
		}
		DynamicMaterial = nullptr;
		DynamicMaterial_Clip = nullptr;

		for (AActor* OutlinedActor : OutlinedActors)
		{
			if (OutlinedActor)
			{
				SetCustomDepth(OutlinedActor, false);
			}
		}
		OutlinedActors.Empty();
	}
	
	Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);
}

void UAO_GameplayAbility_Outline::OnDurationFinished()
{
	EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, false);
}

void UAO_GameplayAbility_Outline::SetMaterialParams(UMaterialInstanceDynamic* InDynamicMaterial)
{
	if (InDynamicMaterial)
	{
		InDynamicMaterial->SetScalarParameterValue(FName("Opacity"), Opacity);
		InDynamicMaterial->SetVectorParameterValue(FName("Color"), Color);
		InDynamicMaterial->SetScalarParameterValue(FName("OutlineOpacity"), OutlineOpacity);
		InDynamicMaterial->SetVectorParameterValue(FName("OutlineColor"), OutlineColor);
	}
}

void UAO_GameplayAbility_Outline::SetCustomDepth(AActor* Target, bool bIsCustomDepth)
{
	TArray<UMeshComponent*> Meshes;
	Target->GetComponents<UMeshComponent>(Meshes);
	for (UMeshComponent* Mesh : Meshes)
	{
		if (Mesh)
		{
			Mesh->SetRenderCustomDepth(bIsCustomDepth);
			if (bIsCustomDepth)
			{
				Mesh->SetCustomDepthStencilValue(240);
			}
		}
	}
}
