// AO_Enemy_HumanoidBase.h

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "AO_Enemy_HumanoidBase.generated.h"

UENUM(BlueprintType)
enum class EEnemyType : uint8
{
	Troll		UMETA(DisplayName = "Troll"),
	Werewolf	UMETA(DisplayName = "Werewolf"),
};

UENUM(BlueprintType)
enum class EEnemyMovementState : uint8
{
	Idle	UMETA(DisplayName = "Idle"),
	Patrol  UMETA(DisplayName = "Patrol"),
	Chase   UMETA(DisplayName = "Chase")
};

UCLASS()
class AO_API AAO_Enemy_HumanoidBase : public ACharacter
{
	GENERATED_BODY()

public:
	AAO_Enemy_HumanoidBase();

	virtual void BeginPlay() override;

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

public:
	UPROPERTY(EditDefaultsOnly, Category = "Enemy")
	EEnemyType EnemyType = EEnemyType::Troll;

	UPROPERTY(ReplicatedUsing = OnRep_MovementState, VisibleAnywhere, Category = "Enemy")
	EEnemyMovementState MovementState = EEnemyMovementState::Idle;

	UFUNCTION()
	void OnRep_MovementState();

	UPROPERTY(Replicated, VisibleAnywhere, Category = "Enemy|Target")
	TObjectPtr<AActor> CurrentTarget;

	UPROPERTY(Replicated, VisibleAnywhere, Category = "Enemy|Target")
	FVector LastSeenLocation = FVector::ZeroVector;

	UPROPERTY(EditDefaultsOnly, Category = "Enemy|Movement")
	float WalkSpeed = 300.0f;

	UPROPERTY(EditDefaultsOnly, Category = "Enemy|Movement")
	float ChaseSpeed = 450.0f;

	UPROPERTY(EditDefaultsOnly, Category = "Enemy|Combat")
	float AttackRange = 150.0f;

	UPROPERTY(Replicated)
	bool bCanAttack = true;

public:
	UFUNCTION(BlueprintCallable, Category = "Enemy|Target")
	void SetCurrentTarget(AActor* NewTarget);

	UFUNCTION(BlueprintCallable, Category = "Enemy|Target")
	void SetLastSeenLocation(const FVector& Location);

	UFUNCTION(BlueprintCallable, Category = "Enemy|Movement")
	void SetMovementState(EEnemyMovementState NewState);

	void ApplyMovementSpeed();

	void SetCanAttack(bool bValue);
};