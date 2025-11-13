// HSJ : AO_PuzzleConditionChecker.h
#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "GameplayTagContainer.h"
#include "AO_PuzzleConditionChecker.generated.h"

/**
 * 퍼즐 조건 정의 구조체
 * 
 * - RequiredTags: 필수로 활성화되어야 하는 태그들
 * - BlockingTags: 활성화되면 조건 실패하는 태그들
 * - OrderedTags: 특정 순서대로 활성화되어야 하는 태그들 (bRequiresOrder=true 시)
 * - TimeLimit: 시간 제한 (초, 0이면 무제한)
 */
USTRUCT(BlueprintType)
struct FPuzzleCondition
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Condition")
	FGameplayTagContainer RequiredTags;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Condition")
	FGameplayTagContainer BlockingTags;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Condition")
	bool bRequiresOrder = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Condition",
		meta=(EditCondition="bRequiresOrder", EditConditionHides))
	TArray<FGameplayTag> OrderedTags;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Condition")
	float TimeLimit = 0.0f;
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnPuzzleCompleted);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnPuzzleFailed);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnPuzzleProgress, float, Progress);

/**
 * 퍼즐 조건 검사기
 * 
 * - 여러 조건 조합 지원 (AND/OR)
 * - 순서 조건 검사 (특정 순서대로 이벤트 발생)
 * - 시간 제한 조건
 * - 차단 태그 (특정 태그 활성화 시 실패)
 * 
 * 1. PuzzleElement가 OnPuzzleEvent() 호출
 * 2. ActiveEvents에 태그 추가 및 조건 검사
 * 3. 모든 조건 만족 시 CompletionTag를 TargetActors에 부여
 * 
 * 사용 예:
 * - 버튼 3개를 특정 순서로 눌러야 문 열림
 * - 레버 2개를 동시에 당겨야 통과
 * - 30초 내에 5개 스위치 모두 활성화
 */
UCLASS(Blueprintable)
class AO_API AAO_PuzzleConditionChecker : public AActor
{
	GENERATED_BODY()

public:
	AAO_PuzzleConditionChecker();

	UFUNCTION(BlueprintCallable, Category="Puzzle")
	void OnPuzzleEvent(FGameplayTag EventTag, AActor* EventInstigator);

	// 퍼즐 이벤트 제거 (토글 시 사용)
	UFUNCTION(BlueprintCallable, Category="Puzzle")
	void RemovePuzzleEvent(FGameplayTag EventTag);

	// 현재 진행도 반환 (0.0 ~ 1.0)
	UFUNCTION(BlueprintPure, Category="Puzzle")
	float GetCompletionProgress() const;

	// 퍼즐 초기화 (다시 시작)
	UFUNCTION(BlueprintCallable, Category="Puzzle")
	void ResetPuzzle();

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Puzzle")
	bool bRequireAllConditions = true; // true: AND, false: OR

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Puzzle")
	TArray<FPuzzleCondition> Conditions; // 조건 목록

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Puzzle")
	TArray<TObjectPtr<AActor>> TargetActors; // 완료 시 CompletionTag 부여할 액터들

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Puzzle")
	FGameplayTag CompletionTag; // 완료 시 부여할 태그

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Puzzle")
	bool bOneTimeCompletion = true; // true: 한 번만 완료 가능

	UPROPERTY(ReplicatedUsing=OnRep_ActiveEvents, BlueprintReadOnly, Category="Puzzle")
	FGameplayTagContainer ActiveEvents; // 현재 활성화된 이벤트 태그들

	UPROPERTY(ReplicatedUsing=OnRep_IsCompleted, BlueprintReadOnly, Category="Puzzle")
	bool bIsCompleted = false;

	UPROPERTY(BlueprintAssignable, Category="Puzzle")
	FOnPuzzleCompleted OnPuzzleCompleted;

	UPROPERTY(BlueprintAssignable, Category="Puzzle")
	FOnPuzzleFailed OnPuzzleFailed;

	UPROPERTY(BlueprintAssignable, Category="Puzzle")
	FOnPuzzleProgress OnPuzzleProgress;

protected:
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	bool CheckAllConditions();
	bool CheckSingleCondition(const FPuzzleCondition& Condition, int32 ConditionIndex);

	void CompletePuzzle();
	void FailPuzzle();

	void StartConditionTimer(int32 ConditionIndex, float Duration);
	void OnConditionTimeout(int32 ConditionIndex);

	UFUNCTION()
	void OnRep_IsCompleted();

	UFUNCTION()
	void OnRep_ActiveEvents();

	UFUNCTION(NetMulticast, Reliable)
	void MulticastPuzzleFailed();

private:
	void ClearAllTimers();

	TMap<int32, TArray<FGameplayTag>> ConditionOrderHistory; // 순서 조건용 히스토리
	TMap<int32, FTimerHandle> ConditionTimerHandles; // 조건별 타이머
};