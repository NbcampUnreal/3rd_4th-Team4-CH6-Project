// AO_BTT_DropItem.cpp

#include "AO/Public/AI/BT/BTTasks/AO_BTT_DropItem.h"
#include "AO/Public/AI/Character/AO_Enemy_Crab.h"
#include "AO/Public/AI/Item/AO_TestPickupItem.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "AIController.h"
#include "TimerManager.h"

UAO_BTT_DropItem::UAO_BTT_DropItem()
{
    NodeName = TEXT("Drop Item");

    RecentlyDroppedTagDuration = 8.0f;

    HasItemKey.AddBoolFilter(this, GET_MEMBER_NAME_CHECKED(UAO_BTT_DropItem, HasItemKey));
    LastDroppedItemKey.AddObjectFilter(this, GET_MEMBER_NAME_CHECKED(UAO_BTT_DropItem, LastDroppedItemKey), AAO_TestPickupItem::StaticClass());
}

EBTNodeResult::Type UAO_BTT_DropItem::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
    UBlackboardComponent* BB = OwnerComp.GetBlackboardComponent();
    const AAIController* AICon = OwnerComp.GetAIOwner();
    AAO_Enemy_Crab* Crab = AICon ? Cast<AAO_Enemy_Crab>(AICon->GetPawn()) : nullptr;

    if (Crab == nullptr || BB == nullptr)
    {
        return EBTNodeResult::Failed;
    }

    AAO_TestPickupItem* PrevHeld = Crab->HeldItem;

    if (PrevHeld == nullptr)
    {
        if (HasItemKey.SelectedKeyName != NAME_None)
        {
            BB->SetValueAsBool(HasItemKey.SelectedKeyName, false);
        }
        return EBTNodeResult::Succeeded;
    }

    if (Crab->HasAuthority())
    {
        Crab->DropItem();
    }
    else
    {
        Crab->ServerDropItem();
    }
    
    if (HasItemKey.SelectedKeyName != NAME_None)
    {
        BB->SetValueAsBool(HasItemKey.SelectedKeyName, false);
    }
    if (LastDroppedItemKey.SelectedKeyName != NAME_None)
    {
        BB->SetValueAsObject(LastDroppedItemKey.SelectedKeyName, PrevHeld);
    }
    
    if (RecentlyDroppedTag.IsValid())
    {
        PrevHeld->AddGameplayTag(RecentlyDroppedTag);
        ScheduleTagRemoval(PrevHeld, RecentlyDroppedTag, RecentlyDroppedTagDuration);
    }

    return EBTNodeResult::Succeeded;
}

void UAO_BTT_DropItem::ScheduleTagRemoval(AAO_TestPickupItem* Item, const FGameplayTag Tag, float Duration) const
{
    if (Item == nullptr)
    {
        return;
    }
    
    FTimerHandle TimerHandle;
    FTimerDelegate Delegate;
    Delegate.BindLambda([Item, Tag]()
    {
        if (IsValid(Item))
        {
            Item->RemoveGameplayTag(Tag);
        }
    });

    if (UWorld* World = Item->GetWorld())
    {
        World->GetTimerManager().SetTimer(TimerHandle, Delegate, Duration, false);
    }
}