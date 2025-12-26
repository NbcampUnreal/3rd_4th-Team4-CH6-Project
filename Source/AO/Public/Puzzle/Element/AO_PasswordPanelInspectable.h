// HSJ : AO_PasswordPanelInspectable.h
#pragma once

#include "CoreMinimal.h"
#include "Puzzle/Element/AO_InspectionPuzzle.h"
#include "AO_PasswordPanelInspectable.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnPasswordCorrect);

UCLASS()
class AO_API AAO_PasswordPanelInspectable : public AAO_InspectionPuzzle
{
	GENERATED_BODY()

public:
	AAO_PasswordPanelInspectable();

	virtual void OnInspectionMeshClicked(UPrimitiveComponent* ClickedComponent) override;

	UFUNCTION(BlueprintImplementableEvent, Category = "Password")
	void OnButtonClickedEvent(UPrimitiveComponent* ButtonComponent);

	UFUNCTION(BlueprintCallable, Category = "Password")
	void NotifyPasswordCorrect();

	// 랜덤 비밀번호 생성
	UFUNCTION(BlueprintCallable, Category = "Password")
	void GenerateRandomPassword();

	UFUNCTION(BlueprintPure, Category = "Password")
	int32 GetCurrentPassword() const { return CurrentPassword; }

	// 각 자릿수 가져오기
	UFUNCTION(BlueprintPure, Category = "Password")
	void GetPasswordDigits(int32& Digit1, int32& Digit2, int32& Digit3, int32& Digit4) const;
	
	UPROPERTY(BlueprintAssignable, Category = "Password")
	FOnPasswordCorrect OnPasswordCorrectEvent;

protected:
	UFUNCTION(NetMulticast, Reliable)
	void MulticastNotifyButtonClicked(UPrimitiveComponent* ButtonComponent);

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

private:
	UPROPERTY(Replicated, BlueprintReadOnly, Category = "Password", meta=(AllowPrivateAccess="true"))
	int32 CurrentPassword = 0;
};