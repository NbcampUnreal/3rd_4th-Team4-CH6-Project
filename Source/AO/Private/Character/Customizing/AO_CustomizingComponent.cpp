// AO_CustomizingComponent.cpp


#include "Character/Customizing/AO_CustomizingComponent.h"

#include "AO_Log.h"
#include "Character/AO_PlayerCharacter.h"
#include "MuCO/CustomizableObject.h"
#include "MuCO/CustomizableSkeletalComponent.h"
#include "Net/UnrealNetwork.h"

UAO_CustomizingComponent::UAO_CustomizingComponent()
{
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
	
	if (IsValid(Instance))
	{
		PlayerCharacter->GetBodyComponent()->SetCustomizableObjectInstance(Instance);
		PlayerCharacter->GetHeadComponent()->SetCustomizableObjectInstance(Instance);
		PlayerCharacter->GetBodyComponent()->UpdateSkeletalMeshAsync();
		PlayerCharacter->GetHeadComponent()->UpdateSkeletalMeshAsync();

		AO_LOG(LogKSH, Log, TEXT("ChangeCharacterMesh: Mesh update requested - Instance: %s"), *Instance->GetName());
	}
	else
	{
		AO_LOG(LogKSH, Error, TEXT("ChangeCharacterMesh: Instance is invalid"));
	}
	PrintCustomizableObjectInstanceMap();
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

	if (AAO_PlayerCharacter* Character = Cast<AAO_PlayerCharacter>(GetOwner()))
	{
		PlayerCharacter = Character;
	}

	for (const TPair<ECharacterMesh, TObjectPtr<UCustomizableObject>>& Pair : CustomizableObjectMap)
	{
		CustomizableObjectInstanceMap.Add(Pair.Key, Pair.Value->CreateInstance());
	}
}

void UAO_CustomizingComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(UAO_CustomizingComponent, CurrentMeshType);
}

void UAO_CustomizingComponent::OnRep_MeshType()
{
	if (TObjectPtr<UCustomizableObjectInstance>* InstancePtr = CustomizableObjectInstanceMap.Find(CurrentMeshType))
	{
		AO_LOG(LogKSH, Log, TEXT("MeshType: %d"), CurrentMeshType);
		ChangeCharacterMesh(*InstancePtr);
		AO_LOG(LogKSH, Log, TEXT("Change Character Mesh Success"));
		AO_LOG(LogKSH, Log, TEXT("CustomizableObjectInstance : %s"), *InstancePtr->Get()->GetName());
	}
}
