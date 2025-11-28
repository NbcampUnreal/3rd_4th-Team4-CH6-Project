// KSJ : AO_AIC_Werewolf.h

#pragma once

#include "CoreMinimal.h"
#include "AI/Controller/AO_AIC_EnemyBase.h"
#include "AI/Manager/AO_Manager_WerewolfPackManager.h" // Enum 사용 위해 필요
#include "AO_AIC_Werewolf.generated.h"

/**
 * 웨어울프 전용 AI Controller
 * 
 * - PackManager와 통신하여 포위/공격 상태 동기화
 * - 시각 감지 -> Howl
 * - 청각(Howl) 수신 -> 포위 합류
 */
UCLASS()
class AO_API AAO_AIC_Werewolf : public AAO_AIC_EnemyBase
{
	GENERATED_BODY()

public:
	AAO_AIC_Werewolf();

	virtual void OnPossess(APawn* InPawn) override;
	virtual void OnUnPossess() override;

	void ReceiveHowlSignal(const FVector& HowlLocation, AActor* TargetActor);

	UFUNCTION(BlueprintCallable, Category = "AI|Werewolf")
	EWerewolfPackState GetCurrentPackState() const;

	UFUNCTION(BlueprintCallable, Category = "AI|Werewolf")
	AActor* GetPackTargetPlayer() const;

	class UAO_Manager_WerewolfPackManager* GetPackManager() const { return PackManager; }

protected:
	virtual void HandleSight(AActor* Actor, const FAIStimulus& Stimulus) override;

private:
	const FName BBKey_TargetActor = FName("TargetActor");
	const FName BBKey_PackState = FName("PackState");
	const FName BBKey_HowlLocation = FName("HowlLocation");

	UPROPERTY()
	TObjectPtr<class UAO_Manager_WerewolfPackManager> PackManager;
};
