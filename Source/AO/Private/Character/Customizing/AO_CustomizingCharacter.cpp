// AO_CustomizingCharacter.cpp

#include "Character/Customizing/AO_CustomizingCharacter.h"

#include "Camera/CameraComponent.h"
#include "Character/Customizing/AO_DummyCustomComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "MuCO/CustomizableSkeletalComponent.h"


AAO_CustomizingCharacter::AAO_CustomizingCharacter()
{
	PrimaryActorTick.bCanEverTick = false;
	bReplicates = false;

	Root = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));
	SetRootComponent(Root);

	SpringArm = CreateDefaultSubobject<USpringArmComponent>(TEXT("SpringArm"));
	SpringArm->TargetArmLength = 200.0f;
	SpringArm->SetupAttachment(Root);

	Camera = CreateDefaultSubobject<UCameraComponent>(TEXT("Camera"));
	Camera->SetupAttachment(SpringArm, USpringArmComponent::SocketName);

	DefaultSkeletalMesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("DefaultSkeletalMesh"));
	DefaultSkeletalMesh->SetupAttachment(Root);

	BaseSkeletalMesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("BaseSkeletalMesh"));
	BaseSkeletalMesh->SetupAttachment(DefaultSkeletalMesh);
	
	BodySkeletalMesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("BodySkeletalMesh"));
	BodySkeletalMesh->SetupAttachment(BaseSkeletalMesh);
	BodyComponent = CreateDefaultSubobject<UCustomizableSkeletalComponent>(TEXT("BodyComponent"));
	BodyComponent->SetupAttachment(BodySkeletalMesh);
	BodyComponent->SetComponentName(FName("Body"));
	BodyComponent->SetIsReplicated(true);
	
	HeadSkeletalMesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("HeadSkeletalMesh"));
	HeadSkeletalMesh->SetupAttachment(BaseSkeletalMesh);
	HeadComponent = CreateDefaultSubobject<UCustomizableSkeletalComponent>(TEXT("HeadComponent"));
	HeadComponent->SetupAttachment(HeadSkeletalMesh);
	HeadComponent->SetComponentName(FName("Head"));
	HeadComponent->SetIsReplicated(true);

	DummyCustomComponent = CreateDefaultSubobject<UAO_DummyCustomComponent>(TEXT("CustomizingComponent"));
	DummyCustomComponent->SetIsReplicated(true);
}