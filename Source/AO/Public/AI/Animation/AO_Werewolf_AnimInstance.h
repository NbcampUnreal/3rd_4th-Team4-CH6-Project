#pragma once

#include "CoreMinimal.h"
#include "AI/Animation/AO_AIAnimInstance.h"
#include "AO_Werewolf_AnimInstance.generated.h"

class AAO_Werewolf;

/**
 * Werewolf AI 애니메이션 인스턴스
 */
UCLASS()
class AO_API UAO_Werewolf_AnimInstance : public UAO_AIAnimInstance
{
	GENERATED_BODY()

public:
	virtual void NativeInitializeAnimation() override;
	virtual void NativeUpdateAnimation(float DeltaSeconds) override;

protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Character")
	TObjectPtr<AAO_Werewolf> WerewolfCharacter;
};

