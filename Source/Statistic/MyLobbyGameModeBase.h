// MyLobbyGameModeBase.h

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"

#include "MyLobbyGameModeBase.generated.h"

class UMyRobbyWidget;
class UMyLobbyReadyWidget;

UCLASS()
class STATISTIC_API AMyLobbyGameModeBase : public AGameModeBase
{
	GENERATED_BODY()

public:
	virtual void BeginPlay() override;

	UPROPERTY(EditDefaultsOnly)
	TSubclassOf<class UUserWidget> LobbyUIClass;

	UPROPERTY(EditDefaultsOnly)
	TSubclassOf<class UUserWidget> LobbyReadyUIClass;

public:
	UPROPERTY()
	class UMyLobbyWidget* LobbyUI;

	UPROPERTY()
	class UMyLobbyReadyWidget* LobbyReadyUI;

	// 클릭된 IP 저장용
	FString EnteredIP;

	FTimerHandle SleepExTimerHandle;

	// Delegate 콜백 함수
	UFUNCTION()
	void HandleStartPressed(const FString& IP);

	void SleepExTimer();

public:
	UFUNCTION()
    void HandleOutPressed();
};