#include "MyRunGimmickTrigger.h"
#include "PlayerCharacter.h"
#include "Kismet/GameplayStatics.h"
#include "Blueprint/UserWidget.h"
#include "MyRunPointActor.h"
#include "MyTriggerTimer.h"

AMyRunGimmickTrigger::AMyRunGimmickTrigger()
{
    PrimaryActorTick.bCanEverTick = true;
}

void AMyRunGimmickTrigger::BeginPlay()
{
    Super::BeginPlay();
}

void AMyRunGimmickTrigger::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
}

void AMyRunGimmickTrigger::Interact(APlayerCharacter* InteractingPlayer)
{
    Super::Interact(InteractingPlayer);

    if (interactionWidgetInstance)
    {
        interactionWidgetInstance->RemoveFromParent();
        interactionWidgetInstance = nullptr;
    }

    if (!RunPointClass || RunPoints.Num() == 0 || !InteractingPlayer) return;

    UWorld* World = GetWorld();
    if (!World) return;

    cachedPlayer = InteractingPlayer;
    cachedController = Cast<APlayerController>(InteractingPlayer->GetController());

    bIsTriggerEnded = false;
    TotalPoints = RunPoints.Num();
    PassedPoints = 0;
    SecondsRemaining = CountdownTime;

    for (AActor* Target : RunPoints)
    {
        if (!Target) continue;

        FVector SpawnLocation = Target->GetActorLocation();
        FRotator SpawnRotation = FRotator::ZeroRotator;

        AActor* RunActor = World->SpawnActor<AActor>(RunPointClass, SpawnLocation, SpawnRotation);
        if (RunActor)
        {
            SpawnedRunPoints.Add(RunActor);

            AMyRunPointActor* RunPoint = Cast<AMyRunPointActor>(RunActor);
            if (RunPoint)
            {
                RunPoint->SetTriggerOwner(this);
            }
        }
    }

    if (RunPoints[0])
    {
        FVector StartLocation = RunPoints[0]->GetActorLocation();
        cachedPlayer->SetActorLocation(StartLocation);
    }

    if (TimerWidgetClass && cachedController)
    {
        TimerWidgetInstance = CreateWidget<UUserWidget>(cachedController, TimerWidgetClass);
        if (TimerWidgetInstance)
        {
            TimerWidgetInstance->AddToViewport();

            UMyTriggerTimer* TimerWidget = Cast<UMyTriggerTimer>(TimerWidgetInstance);
            if (TimerWidget)
            {
                TimerWidget->UpdateTime(SecondsRemaining);
            }
        }
    }

    World->GetTimerManager().SetTimer(CountdownTimerHandle, this, &AMyRunGimmickTrigger::UpdateCountdown, 1.0f, true);
}

void AMyRunGimmickTrigger::UpdateCountdown()
{
    SecondsRemaining--;

    if (TimerWidgetInstance)
    {
        UMyTriggerTimer* TimerWidget = Cast<UMyTriggerTimer>(TimerWidgetInstance);
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

void AMyRunGimmickTrigger::NotifyPointPassed()
{
    if (bIsTriggerEnded) return;

    PassedPoints++;

    if (PassedPoints >= TotalPoints)
    {
        EndTriggerSuccess();
    }
}

void AMyRunGimmickTrigger::EndTriggerSuccess()
{
    if (bIsTriggerEnded) return;
    bIsTriggerEnded = true;

    GetWorld()->GetTimerManager().ClearTimer(CountdownTimerHandle);

    if (TimerWidgetInstance)
    {
        TimerWidgetInstance->RemoveFromParent();
        TimerWidgetInstance = nullptr;
    }

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

void AMyRunGimmickTrigger::EndTriggerFailed()
{
    if (bIsTriggerEnded) return;
    bIsTriggerEnded = true;

    GetWorld()->GetTimerManager().ClearTimer(CountdownTimerHandle);

    if (TimerWidgetInstance)
    {
        TimerWidgetInstance->RemoveFromParent();
        TimerWidgetInstance = nullptr;
    }

    if (cachedPlayer)
    {
        cachedPlayer->bIsInteractionEnd = false;
        cachedPlayer->bIsInteraction = false;
        cachedPlayer->CurrentInteractTarget = nullptr;
    }
}