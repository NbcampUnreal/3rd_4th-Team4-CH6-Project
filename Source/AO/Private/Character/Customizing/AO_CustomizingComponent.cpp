// AO_CustomizingComponent.cpp


#include "Character/Customizing/AO_CustomizingComponent.h"

#include "AO_Log.h"
#include "Character/AO_PlayerCharacter.h"
#include "MuCO/CustomizableObject.h"
#include "MuCO/CustomizableSkeletalComponent.h"
#include "Net/UnrealNetwork.h"

UAO_CustomizingComponent::UAO_CustomizingComponent()
{
	CurrentOptionData.ParameterName = TEXT("");
	CurrentOptionData.OptionName = TEXT("");
	CurrentOptionData.OptionIndex = 0;
}

void UAO_CustomizingComponent::ServerRPC_ChangeCharacter_Implementation(ECharacterMesh MeshType)
{
	CurrentMeshType = MeshType;
	OnRep_MeshType();
}

void UAO_CustomizingComponent::ChangeCharacterMesh(UCustomizableObjectInstance* Instance)
{
	AO_LOG(LogKSH, Log, TEXT("ChangeCharacterMesh called on %s (HasAuthority: %d, IsLocallyControlled: %d)"), 
	*GetName(), PlayerCharacter->HasAuthority(), PlayerCharacter->IsLocallyControlled());

	checkf(Instance, TEXT("Instance is invalid"));

	PlayerCharacter->GetBodyComponent()->SetCustomizableObjectInstance(Instance);
	PlayerCharacter->GetHeadComponent()->SetCustomizableObjectInstance(Instance);
	PlayerCharacter->GetBodyComponent()->UpdateSkeletalMeshAsync();
	PlayerCharacter->GetHeadComponent()->UpdateSkeletalMeshAsync();
	AO_LOG(LogKSH, Log, TEXT("ChangeCharacterMesh: Mesh update requested - Instance: %s"), *Instance->GetName());
	
	PrintCustomizableObjectInstanceMap();
}

void UAO_CustomizingComponent::ServerRPC_ChangeCustomizingOption_Implementation(FOptionNameAndIndex NewOptionData)
{
	CurrentOptionData = NewOptionData;
	OnRep_ChangeOption();
}

UCustomizableObjectInstance* UAO_CustomizingComponent::GetCustomizableObjectInstanceFromMap(ECharacterMesh MeshType) const
{
	return CustomizableObjectInstanceMap[MeshType];
}

ECharacterMesh UAO_CustomizingComponent::GetCurrentMeshType() const
{
	return CurrentMeshType;
}

void UAO_CustomizingComponent::PrintCustomizableObjectInstanceMap()
{
	for (const TPair<ECharacterMesh, TObjectPtr<UCustomizableObjectInstance>>& Pair : CustomizableObjectInstanceMap)
	{
		AO_LOG_NET(LogKSH, Log, TEXT("MeshType: %d, Instance: %s"), Pair.Key, *Pair.Value->GetName());
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

	OnRep_MeshType();
}

void UAO_CustomizingComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(UAO_CustomizingComponent, CurrentMeshType);
	DOREPLIFETIME(UAO_CustomizingComponent, CurrentOptionData);
}

void UAO_CustomizingComponent::OnRep_MeshType()
{
	TObjectPtr<UCustomizableObjectInstance>* InstancePtr = CustomizableObjectInstanceMap.Find(CurrentMeshType);
	checkf(InstancePtr, TEXT("InstancePtr is invalid"));

	if (PlayerCharacter->GetBodyComponent()->GetCustomizableObjectInstance() != *InstancePtr)
	{
		AO_LOG(LogKSH, Log, TEXT("MeshType: %d"), CurrentMeshType);
		ChangeCharacterMesh(*InstancePtr);
		AO_LOG(LogKSH, Log, TEXT("Change Character Mesh Success"));
		AO_LOG(LogKSH, Log, TEXT("CustomizableObjectInstance : %s"), *InstancePtr->Get()->GetName());
	}
	else
	{
		AO_LOG(LogKSH, Log, TEXT("Already this Character Mesh - "
			       "CurrentMesh : %s, ChangeMesh : %s"),
		       *PlayerCharacter->GetBodyComponent()->GetCustomizableObjectInstance()->GetName(),
		       *InstancePtr->Get()->GetName());
	}
}

void UAO_CustomizingComponent::OnRep_ChangeOption()
{
	TObjectPtr<UCustomizableObjectInstance>* InstancePtr = CustomizableObjectInstanceMap.Find(CurrentMeshType);
	checkf(InstancePtr, TEXT("InstancePtr is invalid"));

	if (TObjectPtr<UCustomizableObject> CustomizableObject = (*InstancePtr)->GetCustomizableObject())
	{
		int ParameterIndex = CustomizableObject->FindParameter(CurrentOptionData.ParameterName);
		FString SelectedOptionName = CustomizableObject->GetIntParameterAvailableOption(ParameterIndex, CurrentOptionData.OptionIndex);
		(*InstancePtr)->SetIntParameterSelectedOption(CurrentOptionData.ParameterName, SelectedOptionName, CurrentOptionData.OptionIndex);
		(*InstancePtr)->UpdateSkeletalMeshAsync();
		AO_LOG(LogKSH, Log, TEXT("ChangeMesh : %s"), *InstancePtr->Get()->GetName());
	}
}
