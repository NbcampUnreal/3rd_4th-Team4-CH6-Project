// AO_PlayerController.cpp (장주만)


#include "Player/PlayerController/AO_PlayerController.h"

#include "AO_Log.h"
#include "CommonLoadingScreen/Public/LoadingScreenManager.h"
#include "Online/AO_OnlineSessionSubsystem.h"
#include "Net/VoiceConfig.h"
#include "VoipListenerSynthComponent.h"

void AAO_PlayerController::PreClientTravel(const FString& PendingURL, ETravelType TravelType, bool bIsSeamlessTravel)
{
	AO_LOG(LogJM, Log, TEXT("Start"));
	GEngine->AddOnScreenDebugMessage(-1, 10.f, FColor::Yellow, FString::Printf(TEXT("Client Travel To %s"), *PendingURL), false);	// JM : 디버그용
	
	if (IsLocalController())
	{
		UAO_OnlineSessionSubsystem* OSS = GetGameInstance()->GetSubsystem<UAO_OnlineSessionSubsystem>();
		if (OSS)
		{
			OSS->MuteAllRemoteTalker();	// JM : 안정성을 위해 Mute도 해주기
			OSS->StopVoiceChat();
		}
		else
		{
			AO_ENSURE(false, TEXT("Can't Get OSS"));
		}

		UWorld* World = GetWorld();
		if (World)
		{
			// JM : 실제 에러의 주범인 'VoipListenerSynthComponent'를 모두 찾아 제거
			for (TObjectIterator<UVoipListenerSynthComponent> It; It; ++It)		// TObjectIterator는 Transient 패키지에 숨은 컴포넌트까지 다 찾아냄
			{
				if (It->GetWorld() == World)	// 이 컴포넌트가 현재 파괴되려는 월드와 연결되어 있는지 확인
				{
					It->UnregisterComponent();
					It->DestroyComponent();
					AO_LOG(LogJM, Log, TEXT("Cleanup: Forced Unregister SynthComponent"));
				}
			}

			for (TObjectIterator<UVOIPTalker> It; It; ++It)
			{
				if (It->GetWorld() == World)
				{
					It->Deactivate();
					It->UnregisterComponent();
					It->DestroyComponent();
					AO_LOG(LogJM, Log, TEXT("Cleanup: Forced Destruction of VOIPTalker"));
				}
			}
		}

		ULoadingScreenManager* LSM = GetGameInstance()->GetSubsystem<ULoadingScreenManager>();
		if (LSM)
		{
			LSM->PendingMapName = PendingURL;
		}
		else
		{
			AO_ENSURE(false, TEXT("Loading Screen Manager is nullptr"));
		}

		/* 해당 딜레이는 동작하지 않음
		// 0.8초 뒤에 Client Travel 실행 (컴퓨터 느리면 시간이 더 필요한듯...)
		PendingTravelInfo = { PendingURL, TravelType, bIsSeamlessTravel };	
		FTimerHandle TravelTimer;
		GetWorldTimerManager().SetTimer(TravelTimer, this, &AAO_PlayerController::ExecutePreClientTravel, 10.8f, false);
		return; // 여기서 Super를 호출하지 않고 일단 멈춤*/
	}

	Super::PreClientTravel(PendingURL, TravelType, bIsSeamlessTravel);
	AO_LOG(LogJM, Log, TEXT("End"));
}

/*
void AAO_PlayerController::ClientTravelInternal_Implementation(const FString& URL, enum ETravelType TravelType,
	bool bSeamless, FGuid MapPackageGuid)
{
	AO_LOG(LogJM, Log, TEXT("Start"));

	// TODO : 함수로 분리하기
	// if (IsLocalController())		// Client RPC라서 이런 비교가 필요 없음
	// {
		UAO_OnlineSessionSubsystem* OSS = GetGameInstance()->GetSubsystem<UAO_OnlineSessionSubsystem>();
		if (OSS)
		{
			OSS->MuteAllRemoteTalker();	// JM : 안정성을 위해 Mute도 해주기
			OSS->StopVoiceChat();
		}
		else
		{
			AO_ENSURE(false, TEXT("Can't Get OSS"));
		}

		UWorld* World = GetWorld();
		if (World)
		{
			// JM : 실제 에러의 주범인 'VoipListenerSynthComponent'를 모두 찾아 제거
			for (TObjectIterator<UVoipListenerSynthComponent> It; It; ++It)		// TObjectIterator는 Transient 패키지에 숨은 컴포넌트까지 다 찾아냄
			{
				if (It->GetWorld() == World)	// 이 컴포넌트가 현재 파괴되려는 월드와 연결되어 있는지 확인
				{
					It->UnregisterComponent();
					It->DestroyComponent();
					AO_LOG(LogJM, Log, TEXT("Cleanup: Forced Unregister SynthComponent"));
				}
			}

			for (TObjectIterator<UVOIPTalker> It; It; ++It)
			{
				if (It->GetWorld() == World)
				{
					It->Deactivate();
					It->UnregisterComponent();
					It->DestroyComponent();
					AO_LOG(LogJM, Log, TEXT("Cleanup: Forced Destruction of VOIPTalker"));
				}
			}
		}

		// TODO : 함수로 분리하기
		ULoadingScreenManager* LSM = GetGameInstance()->GetSubsystem<ULoadingScreenManager>();
		if (LSM)
		{
			LSM->PendingMapName = URL;
		}
		else
		{
			AO_ENSURE(false, TEXT("Loading Screen Manager is nullptr"));
		}

		/*
		FTimerDelegate TravelDelegate;
		TravelDelegate.BindUObject(this, &AAO_PlayerController::ExecuteClientTravelInternal, URL, TravelType, bSeamless, MapPackageGuid);
		GetWorldTimerManager().SetTimer(DelayedTravelTimerHandle, TravelDelegate, 10.5f, false);#1#
	// }

	AO_LOG(LogJM, Log, TEXT("End"));
	// Super::ClientTravelInternal_Implementation(URL, TravelType, bSeamless, MapPackageGuid);
}*/

void AAO_PlayerController::CreateSettingsWidgetInstance(const int32 ZOrder, const ESlateVisibility Visibility)
{
	AO_LOG(LogJM, Log, TEXT("Start"));

	if (!AO_ENSURE(!SettingsWidgetInstance, TEXT("Settings Widget Instance is Already Created")))	// 위젯 중복 생성 방지
	{
		return;
	}

	if (!AO_ENSURE(SettingsWidgetClass, TEXT("Settings Widget Class is nullptr")))
	{
		return;
	}

	SettingsWidgetInstance = CreateWidget<UAO_UserWidget>(this, SettingsWidgetClass);
	if (!AO_ENSURE(SettingsWidgetInstance, TEXT("Settings Widget Instance Create Failed")))
	{
		return;
	}

	SettingsWidgetInstance->AddToViewport(ZOrder);
	SettingsWidgetInstance->SetVisibility(Visibility);
	
	AO_LOG(LogJM, Log, TEXT("End"));
}

/*void AAO_PlayerController::ExecutePreClientTravel()
{
	AO_LOG(LogJM, Log, TEXT("Start"));
	Super::PreClientTravel(PendingTravelInfo.URL, PendingTravelInfo.Type, PendingTravelInfo.bSeamless);
	AO_LOG(LogJM, Log, TEXT("End"));
}*/

// TODO: 상수 참조 파라미터화
/*
void AAO_PlayerController::ExecuteClientTravelInternal(FString URL, ETravelType TravelType, bool bSeamless, FGuid MapPackageGuid)
{
	AO_LOG(LogJM, Log, TEXT("Start"));
	Super::ClientTravelInternal_Implementation(URL, TravelType, bSeamless, MapPackageGuid);
	AO_LOG(LogJM, Log, TEXT("End"));
}
*/
