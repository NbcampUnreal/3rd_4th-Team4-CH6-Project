// HSJ: AO_InteractionEffectSettings.h
#pragma once

#include "CoreMinimal.h"
#include "Particles/ParticleSystem.h"
#include "NiagaraSystem.h"
#include "Sound/SoundBase.h"
#include "AO_InteractionEffectSettings.generated.h"

/**
 * 상호작용 이펙트 설정 (VFX + SFX)
 * - Cascade/Niagara VFX
 * - Sound SFX
 * - Transform/Timing 조절
 */

USTRUCT(BlueprintType)
struct FAO_InteractionEffectSettings
{
    GENERATED_BODY()
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="VFX|Effect")
    TObjectPtr<UParticleSystem> CascadeEffect;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="VFX|Effect")
    TObjectPtr<UNiagaraSystem> NiagaraEffect;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="VFX|Transform")
    FVector VFXRelativeLocation = FVector::ZeroVector;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="VFX|Transform")
    FRotator VFXRelativeRotation = FRotator::ZeroRotator;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="VFX|Transform")
    FVector VFXScale = FVector::OneVector;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="VFX|Timing", 
        meta=(ClampMin="0.0", UIMin="0.0"))
    float VFXSpawnDelay = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="VFX|Attachment")
    bool bAttachVFXToActor = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="VFX|Attachment",
        meta=(EditCondition="bAttachVFXToActor", EditConditionHides))
    FName VFXAttachSocketName = NAME_None;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="VFX|Attachment",
        meta=(EditCondition="bAttachVFXToActor", EditConditionHides))
    EAttachmentRule VFXAttachLocationRule = EAttachmentRule::KeepRelative;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="VFX|Attachment",
        meta=(EditCondition="bAttachVFXToActor", EditConditionHides))
    EAttachmentRule VFXAttachRotationRule = EAttachmentRule::KeepRelative;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="VFX|Attachment",
        meta=(EditCondition="bAttachVFXToActor", EditConditionHides))
    EAttachmentRule VFXAttachScaleRule = EAttachmentRule::KeepRelative;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="SFX|Sound")
    TObjectPtr<USoundBase> Sound;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="SFX|Sound", 
        meta=(ClampMin="0.0", UIMin="0.0"))
    float SoundVolumeMultiplier = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="SFX|Sound", 
        meta=(ClampMin="0.0", UIMin="0.0"))
    float SoundPitchMultiplier = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="SFX|Timing", 
        meta=(ClampMin="0.0", UIMin="0.0"))
    float SoundSpawnDelay = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="SFX|Attachment")
    bool bAttachSoundToActor = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="SFX|Attachment",
        meta=(EditCondition="bAttachSoundToActor", EditConditionHides))
    FName SoundAttachSocketName = NAME_None;

    // 유효성 체크
    bool HasVFX() const
    {
        return CascadeEffect != nullptr || NiagaraEffect != nullptr;
    }

    bool HasSFX() const
    {
        return Sound != nullptr;
    }

    bool IsValid() const
    {
        return HasVFX() || HasSFX();
    }
};