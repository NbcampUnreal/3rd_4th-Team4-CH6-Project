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
        // 들고 있던 게 없으면 상태만 동기화
        if (HasItemKey.SelectedKeyName != NAME_None)
        {
            BB->SetValueAsBool(HasItemKey.SelectedKeyName, false);
        }
        return EBTNodeResult::Succeeded;
    }

    if (Crab->HasAuthority())
    {
        Crab->DropItem();   // 서버에서 바로 실행
    }
    else
    {
        Crab->ServerDropItem(); // 이 상황은 거의 없지만 안정성 보장
    }

    // BB 갱신
    if (HasItemKey.SelectedKeyName != NAME_None)
    {
        BB->SetValueAsBool(HasItemKey.SelectedKeyName, false);
    }
    if (LastDroppedItemKey.SelectedKeyName != NAME_None)
    {
        BB->SetValueAsObject(LastDroppedItemKey.SelectedKeyName, PrevHeld);
    }

    // 일시 태그 부여 (EQS GameplayTags Test에서 제외하기 위함)
    if (RecentlyDroppedTag.IsValid())
    {
        // AO_PickupItem 내부에 FGameplayTagContainer가 있고 태그 조작 API가 있다고 가정
        PrevHeld->AddGameplayTag(RecentlyDroppedTag);          // ← 프로젝트 측에 구현 필요
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

    // 태그 제거를 아이템 액터 타이머로 예약
    FTimerHandle TimerHandle;
    FTimerDelegate Delegate;
    Delegate.BindLambda([Item, Tag]()
    {
        if (IsValid(Item))
        {
            Item->RemoveGameplayTag(Tag); // ← 프로젝트 측에 구현 필요
        }
    });

    if (UWorld* World = Item->GetWorld())
    {
        World->GetTimerManager().SetTimer(TimerHandle, Delegate, Duration, false);
    }
}