//KSJ : AO_AICharacterBase

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "AbilitySystemInterface.h"
#include "AI/AO_AITypes.h"
#include "AO_AICharacterBase.generated.h"

class UAbilitySystemComponent;
class UAO_AIAttributeSet;
class UAO_AIMemoryComponent;
class UGameplayAbility;
class UGameplayEffect;
class UAnimMontage;

UCLASS()
class AO_API AAO_AICharacterBase : public ACharacter, public IAbilitySystemInterface
{
	GENERATED_BODY()

public:
	AAO_AICharacterBase();

	virtual UAbilitySystemComponent* GetAbilitySystemComponent() const override;

	UFUNCTION(BlueprintCallable, Category = "AO|AI|Status")
	bool IsStunned() const;

	UFUNCTION(BlueprintCallable, Category = "AO|AI|Status")
	void OnStunBegin();

	UFUNCTION(BlueprintCallable, Category = "AO|AI|Status")
	void OnStunEnd();

	// 기절 몽타주 멀티캐스트 재생 - 모든 클라이언트에서 재생
	UFUNCTION(NetMulticast, Reliable)
	void Multicast_PlayStunMontage(UAnimMontage* Montage, float PlayRate = 1.0f);

	// 기절 몽타주 멀티캐스트 중지
	UFUNCTION(NetMulticast, Reliable)
	void Multicast_StopStunMontage(float BlendOutTime = 0.25f);

	UFUNCTION(BlueprintCallable, Category = "AO|AI|Memory")
	UAO_AIMemoryComponent* GetMemoryComponent() const { return MemoryComponent; }

	UFUNCTION(BlueprintCallable, CallInEditor, Category = "AO|AI|Debug")
	void TestStun();

	UFUNCTION(BlueprintCallable, CallInEditor, Category = "AO|AI|Debug")
	void TestStunEnd();

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "AO|AI|Combat")
	FEnemyAttackConfig GetCurrentAttackConfig() const;
	virtual FEnemyAttackConfig GetCurrentAttackConfig_Implementation() const;

	UFUNCTION(BlueprintCallable, Category = "AO|AI|Combat")
	virtual void SetIsAttacking(bool bAttacking);

	UFUNCTION(BlueprintCallable, Category = "AO|AI|Combat")
	virtual bool IsAttacking() const { return bIsAttacking; }

protected:
	virtual void BeginPlay() override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	void InitializeAbilitySystem();
	void BindDefaultAbilities();
	void BindDefaultEffects();

	virtual void HandleStunBegin();
	virtual void HandleStunEnd();

protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AO|AI|GAS")
	TObjectPtr<UAbilitySystemComponent> AbilitySystemComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AO|AI|GAS")
	TObjectPtr<UAO_AIAttributeSet> AttributeSet;

	UPROPERTY(EditDefaultsOnly, Category = "AO|AI|GAS")
	TArray<TSubclassOf<UGameplayAbility>> DefaultAbilities;

	UPROPERTY(EditDefaultsOnly, Category = "AO|AI|GAS")
	TArray<TSubclassOf<UGameplayEffect>> DefaultEffects;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AO|AI|Memory")
	TObjectPtr<UAO_AIMemoryComponent> MemoryComponent;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "AO|AI|Movement")
	float DefaultMovementSpeed = 300.f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "AO|AI|Movement")
	float AlertMovementSpeed = 500.f;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "AO|AI|Combat")
	bool bIsAttacking = false;

	// 현재 재생 중인 기절 몽타주 (멀티캐스트 중지용)
	UPROPERTY()
	TObjectPtr<UAnimMontage> CurrentStunMontage = nullptr;
};
