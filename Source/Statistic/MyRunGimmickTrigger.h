#pragma once

#include "SESSION.h"
#include "CoreMinimal.h"
#include "MyGimmickTrigger.h"
#include "MyRunGimmickTrigger.generated.h"

class AMyRunPointActor;
class UUserWidget;
class UMyTriggerTimer;

UCLASS()
class STATISTIC_API AMyRunGimmickTrigger : public AMyGimmickTrigger
{
    GENERATED_BODY()

public:
    AMyRunGimmickTrigger();

protected:
    virtual void BeginPlay() override;

public:
    virtual void Tick(float DeltaTime) override;
    virtual void Interact(APlayerCharacter* InteractingPlayer) override;
    virtual void Active() override;
    virtual void End(bool succeed) override;

    UPROPERTY(EditInstanceOnly, Category = "Run")
    TArray<AActor*> RunPoints;

    UPROPERTY(EditAnywhere, Category = "Run")
    TSubclassOf<AActor> RunPointClass;

    UPROPERTY(EditAnywhere, Category = "Run")
    TSubclassOf<AActor> ChestClass;

    UPROPERTY(EditAnywhere, Category = "Run")
    TSubclassOf<UUserWidget> TimerWidgetClass;

    UPROPERTY(EditAnywhere, Category = "Run")
    int32 CountdownTime = 60;

    UPROPERTY(EditAnywhere, Category = "Run")
    int32 ID;

private:
    int32 TotalPoints = 0;
    int32 PassedPoints = 0;
    int32 SecondsRemaining = 0;

    FTimerHandle CountdownTimerHandle;

    UPROPERTY()
    UUserWidget* TimerWidgetInstance = nullptr;

    UPROPERTY()
    TArray<AActor*> SpawnedRunPoints;

    bool bIsTriggerEnded = false;
    

    void UpdateCountdown();
    void EndTriggerSuccess();
    void EndTriggerFailed();

public:
    void NotifyPointPassed();
};