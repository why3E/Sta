#include "MyBoomsGimmickTrigger.h"
#include "PlayerCharacter.h"
#include "Blueprint/UserWidget.h"
#include "MyBombActor.h"
#include "MyTriggerTimer.h"
#include "Kismet/GameplayStatics.h"

AMyBoomsGimmickTrigger::AMyBoomsGimmickTrigger()
{
    PrimaryActorTick.bCanEverTick = true;
}

void AMyBoomsGimmickTrigger::BeginPlay()
{
    Super::BeginPlay();
}

void AMyBoomsGimmickTrigger::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
}

void AMyBoomsGimmickTrigger::Interact(APlayerCharacter* InteractingPlayer)
{
    if(bIsInteraction) return;
    Super::Interact(InteractingPlayer);

    if (interactionWidgetInstance)
    {
        interactionWidgetInstance->RemoveFromParent();
        interactionWidgetInstance = nullptr;
    }

    if (!BombClass || BombSpawnTargets.Num() == 0) return;

    UWorld* World = GetWorld();
    if (!World) return;

    cachedPlayer = InteractingPlayer;
    cachedController = Cast<APlayerController>(InteractingPlayer->GetController());

    bIsTriggerEnded = false;
    TotalBombs = BombSpawnTargets.Num();
    DestroyedBombs = 0;
    SecondsRemaining = CountdownTime;

    // Spawn bombs
    for (AActor* Target : BombSpawnTargets)
    {
        if (!Target) continue;

        FVector SpawnLocation = Target->GetActorLocation();
        FRotator SpawnRotation = FRotator::ZeroRotator;

        AActor* BombActor = World->SpawnActor<AActor>(BombClass, SpawnLocation, SpawnRotation);
        if (BombActor)
        {
            SpawnedBombs.Add(BombActor);

            AMyBombActor* Bomb = Cast<AMyBombActor>(BombActor);
            if (Bomb)
            {
                Bomb->SetTriggerOwner(this);
            }
        }
    }

    // Create timer widget
    if (BombTimerWidgetClass && cachedController)
    {
        BombTimerWidgetInstance = CreateWidget<UUserWidget>(cachedController, BombTimerWidgetClass);
        if (BombTimerWidgetInstance)
        {
            BombTimerWidgetInstance->AddToViewport();

            UMyTriggerTimer* TimerWidget = Cast<UMyTriggerTimer>(BombTimerWidgetInstance);
            if (TimerWidget)
            {
                TimerWidget->UpdateTime(SecondsRemaining);
            }
        }
    }

    // Start countdown
    World->GetTimerManager().SetTimer(CountdownTimerHandle, this, &AMyBoomsGimmickTrigger::UpdateCountdown, 1.0f, true);
}

void AMyBoomsGimmickTrigger::UpdateCountdown()
{
    SecondsRemaining--;

    if (BombTimerWidgetInstance)
    {
        UMyTriggerTimer* TimerWidget = Cast<UMyTriggerTimer>(BombTimerWidgetInstance);
        if (TimerWidget)
        {
            TimerWidget->UpdateTime(SecondsRemaining);
        }
    }

    if (SecondsRemaining <= 0)
    {
        EndTriggerFailed();
    }
}

void AMyBoomsGimmickTrigger::NotifyBombDestroyed()
{
    if (bIsTriggerEnded) return;

    DestroyedBombs++;

    if (DestroyedBombs >= TotalBombs)
    {
        EndTriggerSuccess();
    }
}

void AMyBoomsGimmickTrigger::EndTriggerSuccess()
{
    if (bIsTriggerEnded) return;
    bIsTriggerEnded = true;
    bIsInteraction = false;

    GetWorld()->GetTimerManager().ClearTimer(CountdownTimerHandle);

    if (BombTimerWidgetInstance)
    {
        BombTimerWidgetInstance->RemoveFromParent();
        BombTimerWidgetInstance = nullptr;
    }

    for (AActor* Bomb : SpawnedBombs)
    {
        if (Bomb && !Bomb->IsActorBeingDestroyed())
        {
            Bomb->Destroy();
        }
    }
    SpawnedBombs.Empty();

    if (ChestClass)
    {
        FVector SpawnLocation = GetActorLocation();
        GetWorld()->SpawnActor<AActor>(ChestClass, SpawnLocation, FRotator::ZeroRotator);
    }

    if (cachedPlayer)
    {
        cachedPlayer->bIsInteractionEnd = false;
        cachedPlayer->bIsInteraction = false;
        cachedPlayer->CurrentInteractTarget = nullptr;
    }

    Destroy();
}

void AMyBoomsGimmickTrigger::EndTriggerFailed()
{
    if (bIsTriggerEnded) return;
    bIsTriggerEnded = true;
    bIsInteraction = false;

    GetWorld()->GetTimerManager().ClearTimer(CountdownTimerHandle);

    if (BombTimerWidgetInstance)
    {
        BombTimerWidgetInstance->RemoveFromParent();
        BombTimerWidgetInstance = nullptr;
    }

    for (AActor* Bomb : SpawnedBombs)
    {
        if (Bomb && !Bomb->IsActorBeingDestroyed())
        {
            Bomb->Destroy();
        }
    }
    SpawnedBombs.Empty();

    if (cachedPlayer)
    {
        cachedPlayer->bIsInteractionEnd = false;
        cachedPlayer->bIsInteraction = false;
        cachedPlayer->CurrentInteractTarget = nullptr;
    }
}
