// AO_SpectateWidget.h

#pragma once

#include "CoreMinimal.h"
#include "AO_UserWidget.h"
#include "AO_SpectateWidget.generated.h"

class UAO_HealthWidget;
class UAO_StaminaWidget;

UCLASS()
class AO_API UAO_SpectateWidget : public UAO_UserWidget
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintImplementableEvent, BlueprintCallable)
	void SetSpectatingPlayerName(const FText& InName);
	UFUNCTION(BlueprintImplementableEvent, BlueprintCallable)
	void BeforeSpectatingCharacterChanged(ACharacter* InCharacter);
	UFUNCTION(BlueprintImplementableEvent, BlueprintCallable)
	void AfterSpectatingCharacterChanged(ACharacter* InCharacter);
	
	UFUNCTION(BlueprintCallable, Category = "Spectate")
	void SetSpectatingCharacter(ACharacter* InCharacter);
	
protected:
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UAO_HealthWidget> HealthWidget;
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UAO_StaminaWidget> StaminaWidget;

	UPROPERTY()
	TObjectPtr<ACharacter> SpectatingCharacter;
};
