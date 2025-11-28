// KSJ : AO_AIC_Werewolf.cpp

#include "AI/Controller/AO_AIC_Werewolf.h"
#include "AI/Manager/AO_Manager_WerewolfPackManager.h"
#include "BehaviorTree/BehaviorTree.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Kismet/GameplayStatics.h"
#include "AO_Log.h" 

AAO_AIC_Werewolf::AAO_AIC_Werewolf()
{
}

void AAO_AIC_Werewolf::OnPossess(APawn* InPawn)
{
	Super::OnPossess(InPawn);

	UWorld* World = GetWorld();
	if (World)
	{
		PackManager = World->GetSubsystem<UAO_Manager_WerewolfPackManager>();
		if (PackManager)
		{
			PackManager->RegisterWerewolf(this);
		}
	}

	if (BehaviorTreeAsset)
	{
		RunBehaviorTree(BehaviorTreeAsset);
	}
}

void AAO_AIC_Werewolf::OnUnPossess()
{
	if (PackManager)
	{
		PackManager->UnregisterWerewolf(this);
	}
	Super::OnUnPossess();
}

void AAO_AIC_Werewolf::HandleSight(AActor* Actor, const FAIStimulus& Stimulus)
{
	Super::HandleSight(Actor, Stimulus);

	if (!Stimulus.WasSuccessfullySensed())
	{
		UBlackboardComponent* BB = GetBlackboardComponent();
		if (BB)
		{
			BB->SetValueAsBool("bPlayerVisible", false);
			BB->SetValueAsVector("LastKnownPlayerLocation", Stimulus.StimulusLocation);
		}
		return;
	}
	
	UBlackboardComponent* BB = GetBlackboardComponent();
	if (!BB) return;

	BB->SetValueAsBool("bPlayerVisible", true);
	BB->SetValueAsVector("LastKnownPlayerLocation", Stimulus.StimulusLocation);

	AActor* CurrentTarget = Cast<AActor>(BB->GetValueAsObject(BBKey_TargetActor));
	
	float DistToCurrent = CurrentTarget ? FVector::DistSquared(GetPawn()->GetActorLocation(), CurrentTarget->GetActorLocation()) : FLT_MAX;
	float DistToNew = FVector::DistSquared(GetPawn()->GetActorLocation(), Actor->GetActorLocation());

	if (CurrentTarget != Actor && DistToNew < DistToCurrent)
	{
		AO_LOG(LogKSJ, Log, TEXT("Werewolf switched target to closer one: %s"), *Actor->GetName());

		if (PackManager)
		{
			PackManager->OnTargetSwitched(this, CurrentTarget, Actor);
		}
		BB->SetValueAsObject(BBKey_TargetActor, Actor);
		
		if (PackManager)
		{
			PackManager->ReportPlayerDetection(this, Actor);
		}
	}
	else if (CurrentTarget == nullptr)
	{
		BB->SetValueAsObject(BBKey_TargetActor, Actor);
		if (PackManager)
		{
			PackManager->ReportPlayerDetection(this, Actor);
			BB->SetValueAsEnum(BBKey_PackState, (uint8)GetCurrentPackState());
		}
	}
}

void AAO_AIC_Werewolf::ReceiveHowlSignal(const FVector& HowlLocation, AActor* TargetActor)
{
	AO_LOG(LogKSJ, Log, TEXT("Werewolf %s heard HOWL!"), *GetName());

	UBlackboardComponent* BB = GetBlackboardComponent();
	if (BB)
	{
		BB->SetValueAsObject(BBKey_TargetActor, TargetActor);
		BB->SetValueAsVector(BBKey_HowlLocation, HowlLocation);
		BB->SetValueAsEnum(BBKey_PackState, (uint8)GetCurrentPackState());
	}
}

EWerewolfPackState AAO_AIC_Werewolf::GetCurrentPackState() const
{
	if (PackManager)
	{
		return PackManager->GetPackStateFor(const_cast<AAO_AIC_Werewolf*>(this));
	}
	return EWerewolfPackState::Idle;
}

AActor* AAO_AIC_Werewolf::GetPackTargetPlayer() const
{
	if (PackManager)
	{
		return PackManager->GetSiegeTargetFor(const_cast<AAO_AIC_Werewolf*>(this));
	}
	return nullptr;
}
