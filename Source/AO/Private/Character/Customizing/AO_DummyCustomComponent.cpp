// AO_DummyCustomComponent.cpp


#include "Character/Customizing/AO_DummyCustomComponent.h"
#include "AO_Log.h"
#include "Character/AO_PlayerCharacter.h"
#include "Character/Customizing/AO_CustomizingCharacter.h"
#include "Kismet/GameplayStatics.h"
#include "MuCO/CustomizableObject.h"
#include "MuCO/CustomizableSkeletalComponent.h"
#include "Player/PlayerState/AO_PlayerState.h"

UAO_DummyCustomComponent::UAO_DummyCustomComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
	
	CustomizingData.CharacterMeshType = ECharacterMesh::Elsa;

	CustomizingData.HairOptionData.ParameterName = TEXT("HairStyle");
	CustomizingData.HairOptionData.OptionName = TEXT("Hair01");

	CustomizingData.ClothOptionData.ParameterName = TEXT("ClothType");
	CustomizingData.ClothOptionData.OptionName = TEXT("Glacier");
}

UCustomizableObjectInstance* UAO_DummyCustomComponent::GetCurrentCustomizableObjectInstanceFromMap() const
{
	if (CustomizableObjectInstanceMap.Contains(CustomizingData.CharacterMeshType))
	{
		return CustomizableObjectInstanceMap[CustomizingData.CharacterMeshType];
	}
    
	return nullptr;
}

void UAO_DummyCustomComponent::SaveCustomizingData()
{
	TObjectPtr<UCustomizableObjectInstance> Instance = GetCurrentCustomizableObjectInstanceFromMap();
	checkf(Instance, TEXT("Instance is invalid"));
	
	TObjectPtr<AAO_PlayerCharacter> Player = Cast<AAO_PlayerCharacter>(UGameplayStatics::GetPlayerCharacter(GetWorld(), 0));
	checkf(Player, TEXT("Player is invalid"));

	CustomizingData.HairOptionData.OptionName = Instance->GetIntParameterSelectedOption(CustomizingData.HairOptionData.ParameterName);
	CustomizingData.ClothOptionData.OptionName = Instance->GetIntParameterSelectedOption(CustomizingData.ClothOptionData.ParameterName);

	TObjectPtr<AAO_PlayerState> PlayerState = Cast<AAO_PlayerState>(Player->GetPlayerState());
	checkf(PlayerState, TEXT("PlayerState is invalid"));

	Player->GetCustomizingComponent()->ServerRPC_ChangeCustomizing(CustomizingData);
	PlayerState->CharacterCustomizingData = CustomizingData;
	PlayerState->ServerRPC_SetCharacterCustomizingData(CustomizingData);
}

void UAO_DummyCustomComponent::ChangeCharacterMeshInBlueprint(ECharacterMesh NewMeshType)
{
	CustomizingData.CharacterMeshType = NewMeshType;
	
	TObjectPtr<UCustomizableObjectInstance> Instance = GetCurrentCustomizableObjectInstanceFromMap();
	checkf(Instance, TEXT("Instance is invalid"));

	ChangeCharacterMesh(Instance);
}

void UAO_DummyCustomComponent::ChangeOptionInBlueprint(const FParameterOptionName& NewOptionData)
{
	TObjectPtr<UCustomizableObjectInstance> Instance = GetCurrentCustomizableObjectInstanceFromMap();
	checkf(Instance, TEXT("Instance is invalid"));

	ChangeOption(Instance, NewOptionData);
}

void UAO_DummyCustomComponent::BeginPlay()
{
	Super::BeginPlay();

	TObjectPtr<AAO_CustomizingCharacter> Dummy = Cast<AAO_CustomizingCharacter>(GetOwner());
	checkf(Dummy, TEXT("Character is invalid"));

	CustomizingCharacter = Dummy;
	
	TObjectPtr<AAO_PlayerCharacter> Player = Cast<AAO_PlayerCharacter>(UGameplayStatics::GetPlayerCharacter(GetWorld(), 0));
	checkf(Player, TEXT("Player is invalid"));

	CustomizingData = Player->GetCustomizingComponent()->GetCustomizingData();
	
	for (const TPair<ECharacterMesh, TObjectPtr<UCustomizableObject>>& Pair : CustomizableObjectMap)
	{
		CustomizableObjectInstanceMap.Add(Pair.Key, Pair.Value->CreateInstance());
	}

	ApplyCustomizingData();
}

void UAO_DummyCustomComponent::ChangeCharacterMesh(UCustomizableObjectInstance* Instance)
{
	if (CustomizingCharacter->GetBodyComponent()->GetCustomizableObjectInstance() != Instance)
	{
		CustomizingCharacter->GetBodyComponent()->SetCustomizableObjectInstance(Instance);
		CustomizingCharacter->GetHeadComponent()->SetCustomizableObjectInstance(Instance);
		CustomizingCharacter->GetBodyComponent()->UpdateSkeletalMeshAsync();
		CustomizingCharacter->GetHeadComponent()->UpdateSkeletalMeshAsync();
	}
}

void UAO_DummyCustomComponent::ChangeOption(UCustomizableObjectInstance* Instance, const FParameterOptionName& NewOptionData)
{
	Instance->SetIntParameterSelectedOption(NewOptionData.ParameterName, NewOptionData.OptionName);
	Instance->UpdateSkeletalMeshAsync();
}

void UAO_DummyCustomComponent::ApplyCustomizingData()
{
	if (CustomizableObjectInstanceMap.Num() == 0)
	{
		AO_LOG(LogKSH, Warning, TEXT("ApplyCustomizingData failed: CustomizableObjectInstanceMap is empty"));
		return;
	}
	
	TObjectPtr<UCustomizableObjectInstance> Instance = GetCurrentCustomizableObjectInstanceFromMap();
	checkf(Instance, TEXT("Instance is invalid"));
	
	ChangeCharacterMesh(Instance);

	if (!CustomizingData.HairOptionData.OptionName.IsEmpty())
	{
		ChangeOption(Instance, CustomizingData.HairOptionData);
	}

	if (!CustomizingData.ClothOptionData.OptionName.IsEmpty())
	{
		ChangeOption(Instance, CustomizingData.ClothOptionData);
	}
}