// AO_AIAttributeSet.h

#pragma once

#include "CoreMinimal.h"
#include "AttributeSet.h"
#include "AbilitySystemComponent.h"
#include "AO_AIAttributeSet.generated.h"

// Attribute 접근자 매크로 (플레이어 AttributeSet과 동일한 패턴)
#define ATTRIBUTE_ACCESSORS_BASIC(ClassName, PropertyName) \
	GAMEPLAYATTRIBUTE_PROPERTY_GETTER(ClassName, PropertyName) \
	GAMEPLAYATTRIBUTE_VALUE_GETTER(PropertyName) \
	GAMEPLAYATTRIBUTE_VALUE_SETTER(PropertyName) \
	GAMEPLAYATTRIBUTE_VALUE_INITTER(PropertyName)

/**
 * AI 캐릭터용 Attribute Set
 * - 이동 속도 관련 Attribute
 * - AI는 체력이 없음 (요구사항)
 */
UCLASS()
class AO_API UAO_AIAttributeSet : public UAttributeSet
{
	GENERATED_BODY()

public:
	UAO_AIAttributeSet();

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	virtual void PreAttributeChange(const FGameplayAttribute& Attribute, float& NewValue) override;

	// 이동 속도
	UPROPERTY(BlueprintReadOnly, ReplicatedUsing = OnRep_MovementSpeed, Category = "AO|AI|Attributes")
	FGameplayAttributeData MovementSpeed;
	ATTRIBUTE_ACCESSORS_BASIC(UAO_AIAttributeSet, MovementSpeed)

	// 최대 이동 속도
	UPROPERTY(BlueprintReadOnly, ReplicatedUsing = OnRep_MaxMovementSpeed, Category = "AO|AI|Attributes")
	FGameplayAttributeData MaxMovementSpeed;
	ATTRIBUTE_ACCESSORS_BASIC(UAO_AIAttributeSet, MaxMovementSpeed)

protected:
	UFUNCTION()
	void OnRep_MovementSpeed(const FGameplayAttributeData& OldValue);

	UFUNCTION()
	void OnRep_MaxMovementSpeed(const FGameplayAttributeData& OldValue);
};
