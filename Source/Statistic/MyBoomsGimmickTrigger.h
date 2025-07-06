#pragma once

#include "CoreMinimal.h"
#include "MyGimmickTrigger.h"
#include "MyBoomsGimmickTrigger.generated.h"

class AMyBombActor;

UCLASS()
class STATISTIC_API AMyBoomsGimmickTrigger : public AMyGimmickTrigger
{
    GENERATED_BODY()

public:
    AMyBoomsGimmickTrigger();

protected:
    virtual void BeginPlay() override;

public:
    virtual void Tick(float DeltaTime) override;
    virtual void Interact(APlayerCharacter* InteractingPlayer) override;

    UPROPERTY(EditInstanceOnly, Category = "Boom")
    TArray<AActor*> BombSpawnTargets;

    UPROPERTY(EditAnywhere, Category = "Boom")
    TSubclassOf<AActor> BombClass;

    UPROPERTY(EditAnywhere, Category = "Boom")
    TSubclassOf<AActor> ChestClass;

    UPROPERTY(EditAnywhere, Category = "Boom")
    TSubclassOf<UUserWidget> BombTimerWidgetClass;

    UPROPERTY(EditAnywhere, Category = "Boom")
    int32 CountdownTime = 60;

private:
    int32 TotalBombs = 0;
    int32 DestroyedBombs = 0;
    int32 SecondsRemaining = 0;

    FTimerHandle CountdownTimerHandle;

    UPROPERTY()
    UUserWidget* BombTimerWidgetInstance = nullptr;

    UPROPERTY()
    TArray<AActor*> SpawnedBombs;

    bool bIsTriggerEnded = false;

    void UpdateCountdown();
    void EndTriggerSuccess();
    void EndTriggerFailed();

public:
    void NotifyBombDestroyed();
};
