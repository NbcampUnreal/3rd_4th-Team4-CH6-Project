// AO_PlayerController.cpp (장주만)


#include "Player/PlayerController/AO_PlayerController.h"

#include "AO_Log.h"
#include "CommonLoadingScreen/Public/LoadingScreenManager.h"
#include "Online/AO_OnlineSessionSubsystem.h"
#include "AudioDevice.h"
#include "Net/VoiceConfig.h"
#include "VoipListenerSynthComponent.h"

void AAO_PlayerController::PreClientTravel(const FString& PendingURL, ETravelType TravelType, bool bIsSeamlessTravel)
{
	AO_LOG(LogJM, Log, TEXT("Start"));
	if (IsLocalController())
	{
		UAO_OnlineSessionSubsystem* OSS = GetGameInstance()->GetSubsystem<UAO_OnlineSessionSubsystem>();
		if (OSS)
		{
			OSS->MuteAllRemoteTalker();	// 뮤트 안해도 되던데? (컴퓨터마다 다른가..?)
			OSS->StopVoiceChat();
		}
		else
		{
			AO_ENSURE(false, TEXT("Can't Get OSS"));
		}

		/*JM: 이거 지워도 되나? 테스트해보기 : 지우니까 죽는데? */
		UWorld* CurrentWorld = GetWorld();
		if (CurrentWorld)
		{
			// [중요] 2. 실제 에러의 주범인 'VoipListenerSynthComponent'를 모두 찾아 제거
			// TObjectIterator는 Transient 패키지에 숨은 컴포넌트까지 다 찾아냅니다.
			// 이거 없어도 되지 않아? 아닌가.. 에디터에서는 안도는데, 패키징에서는 이거빼면 에러뜸
			for (TObjectIterator<UVoipListenerSynthComponent> It; It; ++It)
			{
				// 이 컴포넌트가 현재 파괴되려는 월드와 연결되어 있는지 확인
				if (It->GetWorld() == CurrentWorld)
				{
					// It->Stop(); // 재생 중단
					It->UnregisterComponent(); // 엔진 등록 해제 (이게 핵심!)
					It->DestroyComponent(); // 파괴
					AO_LOG(LogJM, Log, TEXT("Cleanup: Forced Unregister SynthComponent"));
				}
			}

			// TODO : JM : 이거 없어도 되나?
			// 3. VOIPTalker 컴포넌트들도 함께 제거
			for (TObjectIterator<UVOIPTalker> It; It; ++It)
			{
				if (It->GetWorld() == CurrentWorld)
				{
					It->Deactivate();
					It->UnregisterComponent();
					It->DestroyComponent();
					AO_LOG(LogJM, Log, TEXT("Cleanup: Forced Destruction of VOIPTalker"));
				}
			}
		}

		/*
		// 2. [테스트용 핵심 코드] 현재 월드의 모든 VOIP Talker 컴포넌트를 찾아서 직접 파괴
		// Iterator를 사용하면 캐릭터를 일일이 참조하지 않아도 월드 내 모든 토커를 잡을 수 있습니다.
		if (UWorld* World = GetWorld())
		{
			for (TObjectIterator<UVOIPTalker> It; It; ++It)
			{
				// 현재 내 월드에 속한 컴포넌트인지 확인
				if (It->GetWorld() == World)
				{
					// 엔진 등록 해제 및 파괴
					// It->RegisterWithPlayerState(nullptr); // PS와 연결 해제 (크래쉬 유발로 코드 삭제)
					It->Deactivate();
					It->UnregisterComponent();
					It->DestroyComponent();
                    
					AO_LOG(LogJM, Log, TEXT("Forced Destruction of VOIPTalker: %s"), *It->GetName());
				}
			}
		}*/

		// 2. 물리적 오디오 장치를 비웁니다. -> 하지마 그냥;;; 소리가 다 꺼지네계속
		/*if (GEngine)
		{
			if (FAudioDeviceHandle AudioDeviceHandle = GEngine->GetMainAudioDevice())
			{
				AudioDeviceHandle->StopAllSounds(true);
				AudioDeviceHandle->Flush(nullptr);
			}
		}*/

		GEngine->AddOnScreenDebugMessage(-1, 10.f, FColor::Yellow, FString::Printf(TEXT("Client Travel To %s"), *PendingURL), false);	// JM : 디버그용

		ULoadingScreenManager* LSM = GetGameInstance()->GetSubsystem<ULoadingScreenManager>();
		if (LSM)
		{
			LSM->PendingMapName = PendingURL;
		}
		else
		{
			GEngine->AddOnScreenDebugMessage(-1, 10.f, FColor::Yellow, FString::Printf(TEXT("no lsm")), false);	// JM : 디버그용
			AO_ENSURE(false, TEXT("Loading Screen Manager is nullptr"));
		}

		// TODO : 0.5초 딜레이 후에 호출하도록
		// 1. 정보를 저장
		PendingTravelInfo = { PendingURL, TravelType, bIsSeamlessTravel };

		// 3. 0.8초 뒤에 진짜 Travel 실행 (컴퓨터 느리면 시간이 더 필요한듯...)
		FTimerHandle TravelTimer;
		GetWorldTimerManager().SetTimer(TravelTimer, this, &AAO_PlayerController::ExecutePreClientTravel, 0.8f, false);
        
		return; // 여기서 Super를 호출하지 않고 일단 멈춤
	}

	Super::PreClientTravel(PendingURL, TravelType, bIsSeamlessTravel);
	AO_LOG(LogJM, Log, TEXT("End"));
}

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

void AAO_PlayerController::ExecutePreClientTravel()
{
	AO_LOG(LogJM, Log, TEXT("Start"));
	Super::PreClientTravel(PendingTravelInfo.URL, PendingTravelInfo.Type, PendingTravelInfo.bSeamless);
	AO_LOG(LogJM, Log, TEXT("End"));
}
