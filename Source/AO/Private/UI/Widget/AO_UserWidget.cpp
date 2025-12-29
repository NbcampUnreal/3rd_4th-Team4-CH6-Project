// AO_UserWidget.cpp (장주만)

#include "UI/Widget/AO_UserWidget.h"

#include "AO_Log.h"
#include "Kismet/GameplayStatics.h"

void UAO_UserWidget::PlayUISoundFromDataTable(FName RowName, UDataTable* SoundDataTable)
{
	if (!SoundDataTable)
	{
		AO_LOG(LogJM, Warning, TEXT("SoundDataTable is Null!"));
		return;
	}

	// 1. Get Data Table Row (블루프린트의 'Get Data Table Row' 노드)
	static const FString ContextString(TEXT("Sound Context"));
	FAO_SoundRow* FoundRow = SoundDataTable->FindRow<FAO_SoundRow>(RowName, ContextString);

	if (FoundRow)
	{
		// 2. Row Found 시점: Play Sound 2D 실행
		if (FoundRow->SoundAsset)
		{
			// IsUISound 체크박스 설정과 동일하게 하려면 5번째 인자에 true 전달
			UGameplayStatics::PlaySound2D(
				GetWorld(), 
				FoundRow->SoundAsset, 
				FoundRow->VolumeMultiplier, 
				FoundRow->PitchMultiplier, 
				0.0f,    // Start Time
				nullptr, // Concurrency Settings
				nullptr, // Owning Actor
				true     // Is UISound (블루프린트에서 체크되어 있던 부분)
			);
		}
	}
	else
	{
		// 3. Row Not Found 시점: Print Text 실행
		AO_LOG(LogJM, Warning, TEXT("Row Not Found! (%s)"), *RowName.ToString());
        
		// 화면에 직접 출력하고 싶을 경우
		GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, 
			FString::Printf(TEXT("Row Not Found! (%s)"), *RowName.ToString()));
	}
}
