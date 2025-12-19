// AO_CameraManagerComponent.cpp

#include "Player/Camera/AO_CameraManagerComponent.h"

#include "AO_Log.h"
#include "Camera/CameraComponent.h"
#include "GameFramework/SpringArmComponent.h"

UAO_CameraManagerComponent::UAO_CameraManagerComponent()
{
	PrimaryComponentTick.bCanEverTick = true;

	State_Dead		= FGameplayTag::RequestGameplayTag(FName("Status.Death"));
	State_Sprint	= FGameplayTag::RequestGameplayTag(FName("Status.Action.Sprint"));
	State_Traversal = FGameplayTag::RequestGameplayTag(FName("Status.Action.Traversal"));
	
	Camera_Default   = FGameplayTag::RequestGameplayTag(FName("Camera.Default"));
	Camera_Dead      = FGameplayTag::RequestGameplayTag(FName("Camera.Dead"));
	Camera_Sprint    = FGameplayTag::RequestGameplayTag(FName("Camera.Sprint"));
	Camera_Traversal = FGameplayTag::RequestGameplayTag(FName("Camera.Traversal"));
}

void UAO_CameraManagerComponent::TickComponent(float DeltaTime, enum ELevelTick TickType,
	FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
	
	if (!Camera || !SpringArm)
	{
		return;
	}
	
	UpdateRequests(DeltaTime);

	const FAO_CameraSettings* Target = ChooseTargetProfile();
	if (!Target)
	{
		if (const FAO_CameraSettings* Default = ProfileDB->FindByTag(Camera_Default))
		{
			ApplyProfileBlended(*Default, DeltaTime);
		}
		return;
	}
	
	ApplyProfileBlended(*Target, DeltaTime);
}

void UAO_CameraManagerComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	if (ASC)
	{
		for (const FDelegateHandle& Handle : TagEventHandles)
		{
			ASC->RegisterGameplayTagEvent(State_Dead, EGameplayTagEventType::NewOrRemoved).Remove(Handle);
			ASC->RegisterGameplayTagEvent(State_Sprint, EGameplayTagEventType::NewOrRemoved).Remove(Handle);
			ASC->RegisterGameplayTagEvent(State_Traversal, EGameplayTagEventType::NewOrRemoved).Remove(Handle);
		}
	}
	TagEventHandles.Reset();
	ASC = nullptr;
	
	Super::EndPlay(EndPlayReason);
}

void UAO_CameraManagerComponent::BindCameraComponents(USpringArmComponent* InSpringArm, UCameraComponent* InCamera)
{
	SpringArm = InSpringArm;
	Camera = InCamera;

	if (SpringArm)
	{
		CurArmLength = SpringArm->TargetArmLength;
		CurSocketOffset = SpringArm->SocketOffset;
		bCurEnableLag = SpringArm->bEnableCameraLag;
		CurLagSpeed = SpringArm->CameraLagSpeed;
	}

	if (Camera)
	{
		CurFOV = Camera->FieldOfView;
	}
}

void UAO_CameraManagerComponent::BindToASC(UAbilitySystemComponent* InASC)
{
	ASC = InASC;
	TagEventHandles.Reset();

	checkf(ASC, TEXT("ASC is null"));

	const int32 DeadTagCount      = ASC->GetTagCount(State_Dead);
	const int32 SprintTagCount    = ASC->GetTagCount(State_Sprint);
	const int32 TraversalTagCount = ASC->GetTagCount(State_Traversal);

	OnStateTagChanged(State_Dead, DeadTagCount);
	OnStateTagChanged(State_Sprint, SprintTagCount);
	OnStateTagChanged(State_Traversal, TraversalTagCount);

	TagEventHandles.Add(
		ASC->RegisterGameplayTagEvent(State_Dead, EGameplayTagEventType::NewOrRemoved)
		.AddUObject(this, &UAO_CameraManagerComponent::OnStateTagChanged));
	TagEventHandles.Add(
		ASC->RegisterGameplayTagEvent(State_Sprint, EGameplayTagEventType::NewOrRemoved)
		.AddUObject(this, &UAO_CameraManagerComponent::OnStateTagChanged));
	TagEventHandles.Add(
		ASC->RegisterGameplayTagEvent(State_Traversal, EGameplayTagEventType::NewOrRemoved)
		.AddUObject(this, &UAO_CameraManagerComponent::OnStateTagChanged));
}

void UAO_CameraManagerComponent::OnStateTagChanged(FGameplayTag Tag, int32 NewCount)
{
	const bool bOn = (NewCount > 0);

	if (Tag == State_Dead)
	{
		if (bOn)
		{
			PushCameraState(Camera_Dead);
		}
		else
		{
			PopCameraState(Camera_Dead);
		}
	}
	else if (Tag == State_Sprint)
	{
		if (bOn)
		{
			PushCameraState(Camera_Sprint);
		}
		else
		{
			PopCameraState(Camera_Sprint);
		}
	}
	else if (Tag == State_Traversal)
	{
		if (bOn)
		{
			PushCameraState(Camera_Traversal);
		}
		else
		{
			PopCameraState(Camera_Traversal);
		}
	}
}

void UAO_CameraManagerComponent::PushCameraState(const FGameplayTag& CameraTag)
{
	checkf(ProfileDB, TEXT("ProfileDB is null"));

	const FAO_CameraSettings* Settings = ProfileDB->FindByTag(CameraTag);
	checkf(Settings, TEXT("Failed to find CameraSettings by CameraTag"));

	FAO_CameraRequest& Request = Requests.FindOrAdd(CameraTag);
	Request.Tag = CameraTag;
	Request.Priority = Settings->Priority;
	Request.BlendIn = FMath::Max(0.01f, Settings->BlendInTime);
	Request.BlendOut = FMath::Max(0.01f, Settings->BlendOutTime);
	Request.bActive = true;
}

void UAO_CameraManagerComponent::PopCameraState(const FGameplayTag& CameraTag)
{
	if (FAO_CameraRequest* Request = Requests.Find(CameraTag))
	{
		Request->bActive = false;
	}
}

void UAO_CameraManagerComponent::UpdateRequests(float DeltaTime)
{
	TArray<FGameplayTag> ToRemove;

	for (auto& It : Requests)
	{
		FAO_CameraRequest& CurRequest = It.Value;

		const float Speed = CurRequest.bActive ? (1.f / CurRequest.BlendIn) : (1.f / CurRequest.BlendOut);
		const float Dir = CurRequest.bActive ? 1.f : -1.f;
		CurRequest.Weight = FMath::Clamp(CurRequest.Weight + Dir * Speed * DeltaTime, 0.f, 1.f);

		if (!CurRequest.bActive && CurRequest.Weight <= 0.f)
		{
			ToRemove.Add(It.Key);
		}
	}

	for (const auto& Tag : ToRemove)
	{
		Requests.Remove(Tag);
	}
}

const FAO_CameraSettings* UAO_CameraManagerComponent::ChooseTargetProfile() const
{
	const FAO_CameraSettings* Best = nullptr;
	int32 BestPriority = -1;
	float BestWeight = 0.f;

	for (const auto& It : Requests)
	{
		const FAO_CameraRequest& Request = It.Value;
		if (Request.Weight <= 0.f)
		{
			continue;
		}

		const FAO_CameraSettings* Profile = ProfileDB->FindByTag(Request.Tag);
		if (!Profile)
		{
			continue;
		}

		if (Profile->Priority > BestPriority
			|| (Profile->Priority == BestPriority && Request.Weight > BestWeight))
		{
			BestPriority = Profile->Priority;
			BestWeight = Request.Weight;
			Best = Profile;
		}
	}

	return Best;
}

void UAO_CameraManagerComponent::ApplyProfileBlended(const FAO_CameraSettings& Target, float DeltaTime)
{
	const float InterpSpeed = 12.f;

	CurArmLength = FMath::FInterpTo(CurArmLength, Target.TargetArmLength, DeltaTime, InterpSpeed);
	CurSocketOffset = FMath::VInterpTo(CurSocketOffset, Target.SocketOffset, DeltaTime, InterpSpeed);
	CurFOV = FMath::FInterpTo(CurFOV, Target.FOV, DeltaTime, InterpSpeed);

	bCurEnableLag = Target.bEnableCameraLag;
	CurLagSpeed = Target.CameraLagSpeed;

	SpringArm->TargetArmLength = CurArmLength;
	SpringArm->SocketOffset = CurSocketOffset;
	SpringArm->bEnableCameraLag = bCurEnableLag;
	SpringArm->CameraLagSpeed = CurLagSpeed;

	Camera->SetFieldOfView(CurFOV);
}