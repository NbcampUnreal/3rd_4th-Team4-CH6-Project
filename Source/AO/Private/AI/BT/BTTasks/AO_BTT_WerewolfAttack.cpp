// KSJ : AO_BTT_WerewolfAttack.cpp

#include "AI/BT/BTTasks/AO_BTT_WerewolfAttack.h"
#include "AI/Controller/AO_AIC_Werewolf.h"
#include "AI/Character/AO_Enemy_Werewolf.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "AbilitySystemComponent.h"
#include "AbilitySystemInterface.h"
#include "AbilitySystemBlueprintLibrary.h"
#include "AI/GAS/Effect/AO_GameplayEffect_WerewolfAttack.h"
#include "AO_Log.h"

static void ApplyWerewolfDamage(UBehaviorTreeComponent& OwnerComp, AAO_Enemy_Werewolf* Wolf)
{
    UBlackboardComponent* BB = OwnerComp.GetBlackboardComponent();
    if (BB)
    {
        AActor* Target = Cast<AActor>(BB->GetValueAsObject("TargetPlayer"));
        
        if (Target)
        {
            UAbilitySystemComponent* TargetASC = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(Target);
            UAbilitySystemComponent* SourceASC = Wolf->GetAbilitySystemComponent();
            
            if (TargetASC && SourceASC)
            {
                FGameplayEffectContextHandle ContextHandle = SourceASC->MakeEffectContext();
                ContextHandle.AddSourceObject(Wolf);
                
                FGameplayEffectSpecHandle SpecHandle = SourceASC->MakeOutgoingSpec(UAO_GameplayEffect_WerewolfAttack::StaticClass(), 1.0f, ContextHandle);
                if (SpecHandle.IsValid())
                {
                    SourceASC->ApplyGameplayEffectSpecToTarget(*SpecHandle.Data.Get(), TargetASC);
                    AO_LOG(LogKSJ, Log, TEXT("[BTT_Attack] Applied Damage to %s"), *Target->GetName());
                }
            }
        }   
        
        BB->SetValueAsBool("bPlayerVisible", true);
    }
}

UAO_BTT_WerewolfAttack::UAO_BTT_WerewolfAttack()
{
	NodeName = "Werewolf Attack";
}

EBTNodeResult::Type UAO_BTT_WerewolfAttack::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	AAO_AIC_Werewolf* AIC = Cast<AAO_AIC_Werewolf>(OwnerComp.GetAIOwner());
	if (!AIC) return EBTNodeResult::Failed;

	AAO_Enemy_Werewolf* Wolf = Cast<AAO_Enemy_Werewolf>(AIC->GetPawn());
	if (!Wolf) return EBTNodeResult::Failed;

    AO_LOG(LogKSJ, Log, TEXT("[BTT_Attack] START Attack!"));

	Wolf->PerformAttack();

    UBlackboardComponent* BB = OwnerComp.GetBlackboardComponent();
    if (BB)
    {
        BB->SetValueAsBool("bPlayerVisible", true);
    }

	return EBTNodeResult::Succeeded;
}

void UAO_BTT_WerewolfAttack::TickTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds)
{
}

void UAO_BTT_WerewolfAttack::OnMontageEnded(UAnimMontage* Montage, bool bInterrupted, UBehaviorTreeComponent* OwnerComp)
{
}
