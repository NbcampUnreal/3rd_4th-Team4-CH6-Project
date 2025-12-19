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
	CurrentOptionData.ParameterName = TEXT("");
	CurrentOptionData.OptionName = TEXT("");

	CustomizingData.CharacterMeshType = ECharacterMesh::Elsa;

	CustomizingData.HairOptionData.ParameterName = TEXT("HairStyle");
	CustomizingData.HairOptionData.OptionName = TEXT("Hair01");

	CustomizingData.ClothOptionData.ParameterName = TEXT("ClothType");
	CustomizingData.ClothOptionData.OptionName = TEXT("Glacier");
}

void UAO_CustomizingComponent::ServerRPC_ChangeCharacter_Implementation(ECharacterMesh MeshType)
{
	CurrentMeshType = MeshType;
	ChangeCharacterMesh();
}

void UAO_CustomizingComponent::ServerRPC_ChangeCustomizingOption_Implementation(FParameterOptionName NewOptionData)
{
	CurrentOptionData = NewOptionData;
	ChangeOption();
}

UCustomizableObjectInstance* UAO_CustomizingComponent::GetCustomizableObjectInstanceFromMap(ECharacterMesh MeshType) const
{
	return CustomizableObjectInstanceMap[MeshType];
}

ECharacterMesh UAO_CustomizingComponent::GetCurrentMeshType() const
{
	return CurrentMeshType;
}

void UAO_CustomizingComponent::SaveCustomizingDataToPlayerState()
{
	TObjectPtr<UCustomizableObjectInstance> Instance = GetCustomizableObjectInstanceFromMap(CurrentMeshType);
	checkf(Instance, TEXT("Instance is invalid"));
	
	CustomizingData.CharacterMeshType = CurrentMeshType;
	CustomizingData.HairOptionData.OptionName = Instance->GetIntParameterSelectedOption(CustomizingData.HairOptionData.ParameterName);
	CustomizingData.ClothOptionData.OptionName = Instance->GetIntParameterSelectedOption(CustomizingData.ClothOptionData.ParameterName);

	TObjectPtr<AAO_PlayerState> PlayerState = Cast<AAO_PlayerState>(PlayerCharacter->GetPlayerState());
	checkf(PlayerState, TEXT("PlayerState is invalid"));
	
	PlayerState->CharacterCustomizingData = CustomizingData;
	PlayerState->ServerRPC_SetCharacterCustomizingData(CustomizingData);
	PrintPSCustomizingData();
}

void UAO_CustomizingComponent::PrintCustomizableObjectInstanceMap()
{
	for (const TPair<ECharacterMesh, TObjectPtr<UCustomizableObjectInstance>>& Pair : CustomizableObjectInstanceMap)
	{
		AO_LOG_NET(LogKSH, Log, TEXT("MeshType: %d, Instance: %s"), Pair.Key, *Pair.Value->GetName());
	}
}

void UAO_CustomizingComponent::PrintPSCustomizingData()
{
	TObjectPtr<AAO_PlayerState> PlayerState = Cast<AAO_PlayerState>(PlayerCharacter->GetPlayerState());

	if (PlayerState)
	{
		AO_LOG(LogKSH, Log, TEXT("PS - CustomizingData: MeshType: %d,"
			       "HairOptionData: ParameterName: %s, OptionName: %s,"
			       "ClothOptionData: ParameterName: %s, OptionName: %s"),
		       PlayerState->CharacterCustomizingData.CharacterMeshType,
		       *PlayerState->CharacterCustomizingData.HairOptionData.ParameterName,
		       *PlayerState->CharacterCustomizingData.HairOptionData.OptionName,
		       *PlayerState->CharacterCustomizingData.ClothOptionData.ParameterName,
		       *PlayerState->CharacterCustomizingData.ClothOptionData.OptionName);
	}
	else
	{
		GetWorld()->GetTimerManager().SetTimerForNextTick(this, &UAO_CustomizingComponent::PrintPSCustomizingData);
	}
}

void UAO_CustomizingComponent::BeginPlay()
{
	Super::BeginPlay();

	AAO_PlayerCharacter* Character = Cast<AAO_PlayerCharacter>(GetOwner());
	checkf(Character, TEXT("Character is invalid"));

	PlayerCharacter = Character;

	for (const TPair<ECharacterMesh, TObjectPtr<UCustomizableObject>>& Pair : CustomizableObjectMap)
	{
		CustomizableObjectInstanceMap.Add(Pair.Key, Pair.Value->CreateInstance());
	}

	PrintPSCustomizingData();
	LoadCustomizingDataFromPlayerState();
}

void UAO_CustomizingComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(UAO_CustomizingComponent, CurrentMeshType);
	DOREPLIFETIME(UAO_CustomizingComponent, CurrentOptionData);
}

void UAO_CustomizingComponent::OnRep_CurrentMeshType()
{
	ChangeCharacterMesh();
}

void UAO_CustomizingComponent::ChangeCharacterMesh()
{
	AO_LOG(LogKSH, Log, TEXT("ChangeCharacterMesh called on %s (HasAuthority: %d, IsLocallyControlled: %d)"), 
	*GetName(), PlayerCharacter->HasAuthority(), PlayerCharacter->IsLocallyControlled());

	TObjectPtr<UCustomizableObjectInstance> Instance = GetCustomizableObjectInstanceFromMap(CurrentMeshType);
	checkf(Instance, TEXT("Instance is invalid"));

	if (PlayerCharacter->GetBodyComponent()->GetCustomizableObjectInstance() != Instance)
	{
		PlayerCharacter->GetBodyComponent()->SetCustomizableObjectInstance(Instance);
		PlayerCharacter->GetHeadComponent()->SetCustomizableObjectInstance(Instance);
		PlayerCharacter->GetBodyComponent()->UpdateSkeletalMeshAsync();
		PlayerCharacter->GetHeadComponent()->UpdateSkeletalMeshAsync();
		AO_LOG(LogKSH, Log, TEXT("ChangeCharacterMesh: Mesh update requested - Instance: %s"), *Instance->GetName());
	}
	else
	{
		AO_LOG(LogKSH, Log, TEXT("Already this Character Mesh - "
				   "CurrentMesh : %s, ChangeMesh : %s"),
			   *PlayerCharacter->GetBodyComponent()->GetCustomizableObjectInstance()->GetName(),
			   *Instance->GetName());
	}
	
	PrintCustomizableObjectInstanceMap();
}

void UAO_CustomizingComponent::OnRep_CurrentOptionData()
{
	ChangeOption();
}

void UAO_CustomizingComponent::ChangeOption()
{
	TObjectPtr<UCustomizableObjectInstance> Instance = GetCustomizableObjectInstanceFromMap(CurrentMeshType);
	checkf(Instance, TEXT("Instance is invalid"));

	Instance->SetIntParameterSelectedOption(CurrentOptionData.ParameterName, CurrentOptionData.OptionName);
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

	if (CustomizingData.CharacterMeshType != CurrentMeshType)
	{
		CurrentMeshType = CustomizingData.CharacterMeshType;
	}
	
	ChangeCharacterMesh();

	TObjectPtr<UCustomizableObjectInstance> Instance = GetCustomizableObjectInstanceFromMap(CurrentMeshType);
	checkf(Instance, TEXT("Instance is invalid"));

	if (!CustomizingData.HairOptionData.OptionName.IsEmpty())
	{
		Instance->SetIntParameterSelectedOption(CustomizingData.HairOptionData.ParameterName, CustomizingData.HairOptionData.OptionName);
	}

	if (!CustomizingData.ClothOptionData.OptionName.IsEmpty())
	{
		Instance->SetIntParameterSelectedOption(CustomizingData.ClothOptionData.ParameterName, CustomizingData.ClothOptionData.OptionName);
	}

	Instance->UpdateSkeletalMeshAsync();
}
