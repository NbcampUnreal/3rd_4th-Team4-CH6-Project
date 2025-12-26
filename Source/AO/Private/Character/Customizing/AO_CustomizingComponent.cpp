// AO_CustomizingComponent.cpp


#include "Character/Customizing/AO_CustomizingComponent.h"

#include "AO_Log.h"
#include "Character/AO_PlayerCharacter.h"
#include "MuCO/CustomizableObject.h"
#include "MuCO/CustomizableSkeletalComponent.h"
#include "Net/UnrealNetwork.h"
#include "Player/PlayerState/AO_PlayerState.h"

UAO_CustomizingComponent::UAO_CustomizingComponent()
{
	PrimaryComponentTick.bCanEverTick = false;

	CustomizingData.CharacterMeshType = ECharacterMesh::Elsa;

	CustomizingData.HairOptionData.ParameterName = TEXT("HairStyle");
	CustomizingData.HairOptionData.OptionName = TEXT("Hair01");

	CustomizingData.ClothOptionData.ParameterName = TEXT("ClothType");
	CustomizingData.ClothOptionData.OptionName = TEXT("Glacier");
}

void UAO_CustomizingComponent::ServerRPC_ChangeCustomizing_Implementation(const FCustomizingData& NewCustomizingData)
{
	CustomizingData = NewCustomizingData;
	ApplyCustomizingData();
}

TObjectPtr<UCustomizableObjectInstance> UAO_CustomizingComponent::GetCustomizableObjectInstanceFromMap(ECharacterMesh MeshType) const
{
	if (CustomizableObjectInstanceMap.Contains(MeshType))
	{
		return CustomizableObjectInstanceMap[MeshType];
	}
    
	return nullptr;
}

const FCustomizingData& UAO_CustomizingComponent::GetCustomizingData() const
{
	return CustomizingData;
}

void UAO_CustomizingComponent::BeginPlay()
{
	Super::BeginPlay();

	TObjectPtr<AAO_PlayerCharacter> Character = Cast<AAO_PlayerCharacter>(GetOwner());
	checkf(Character, TEXT("Character is invalid"));

	PlayerCharacter = Character;

	for (const TPair<ECharacterMesh, TObjectPtr<UCustomizableObject>>& Pair : CustomizableObjectMap)
	{
		CustomizableObjectInstanceMap.Add(Pair.Key, Pair.Value->CreateInstance());
	}

	LoadCustomizingDataFromPlayerState();
}

void UAO_CustomizingComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(UAO_CustomizingComponent, CustomizingData);
}

void UAO_CustomizingComponent::OnRep_CustomizingData()
{
	ApplyCustomizingData();
}

void UAO_CustomizingComponent::ChangeCharacterMesh(UCustomizableObjectInstance* Instance)
{
	if (PlayerCharacter->GetBodyComponent()->GetCustomizableObjectInstance() != Instance)
	{
		PlayerCharacter->GetBodyComponent()->SetCustomizableObjectInstance(Instance);
		PlayerCharacter->GetHeadComponent()->SetCustomizableObjectInstance(Instance);
		PlayerCharacter->GetBodyComponent()->UpdateSkeletalMeshAsync();
		PlayerCharacter->GetHeadComponent()->UpdateSkeletalMeshAsync();
	}
}

void UAO_CustomizingComponent::ChangeOption(UCustomizableObjectInstance* Instance, const FParameterOptionName& NewOptionData)
{
	Instance->SetIntParameterSelectedOption(NewOptionData.ParameterName, NewOptionData.OptionName);
	Instance->UpdateSkeletalMeshAsync();
}

void UAO_CustomizingComponent::LoadCustomizingDataFromPlayerState()
{
	TObjectPtr<AAO_PlayerState> PS = Cast<AAO_PlayerState>(PlayerCharacter->GetPlayerState());

	if (PS)
	{
		CustomizingData = PS->CharacterCustomizingData;
		ApplyCustomizingData();
	}
	else
	{
		GetWorld()->GetTimerManager().SetTimerForNextTick(this, &UAO_CustomizingComponent::LoadCustomizingDataFromPlayerState);
	}
}

void UAO_CustomizingComponent::ApplyCustomizingData()
{
	if (CustomizableObjectInstanceMap.Num() == 0)
	{
		AO_LOG(LogKSH, Warning, TEXT("ApplyCustomizingData failed: CustomizableObjectInstanceMap is empty"));
		return;
	}
	
	TObjectPtr<UCustomizableObjectInstance> Instance = GetCustomizableObjectInstanceFromMap(CustomizingData.CharacterMeshType);
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