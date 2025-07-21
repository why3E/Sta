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

    g_c_objects[ID] = this;
}

void AMyRunGimmickTrigger::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
}

void AMyRunGimmickTrigger::Interact(APlayerCharacter* InteractingPlayer)
{
    if (bIsInteraction) return;

    Super::Interact(InteractingPlayer);

    gimmick_start_packet p;
    p.packet_size = sizeof(gimmick_start_packet);
    p.packet_type = C2H_GIMMICK_START_PACKET;
    p.object_id = ID;
    InteractingPlayer->do_send(&p);
}

void AMyRunGimmickTrigger::Active() {
    if (interactionWidgetInstance)
    {
        interactionWidgetInstance->RemoveFromParent();
        interactionWidgetInstance = nullptr;
    }

    if (!RunPointClass || RunPoints.Num() == 0) return;

    UWorld* World = GetWorld();

    if (!World) return;

    for (const auto& player : g_c_players) {
        if (player->get_is_player()) {
            cachedPlayer = player;
            cachedController = Cast<APlayerController>(player->GetController());
            break;
        }
    }

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

                RunPoint->set_id(g_c_object_id++);
                g_c_objects[RunPoint->get_id()] = RunPoint;
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

void AMyRunGimmickTrigger::End(bool succeed) {
    if (succeed) {
        EndTriggerSuccess();
    } else {
        EndTriggerFailed();
    }
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

    if (g_is_host) {
        if (SecondsRemaining <= 0) {
            gimmick_end_packet p;
            p.packet_size = sizeof(gimmick_end_packet);
            p.packet_type = C2H_GIMMICK_END_PACKET;
            p.object_id = ID;
            p.succeed = false;
            cachedPlayer->do_send(&p);
        }
    }
}

void AMyRunGimmickTrigger::NotifyPointPassed()
{
    if (g_is_host) {
        if (bIsTriggerEnded) { return; }

        PassedPoints++;

        if (PassedPoints >= TotalPoints) {
            gimmick_end_packet p;
            p.packet_size = sizeof(gimmick_end_packet);
            p.packet_type = C2H_GIMMICK_END_PACKET;
            p.object_id = ID;
            p.succeed = true;
            cachedPlayer->do_send(&p);
        }
    }
}

void AMyRunGimmickTrigger::EndTriggerSuccess()
{
    if (bIsTriggerEnded) return;
    bIsTriggerEnded = true;
    bIsInteraction = false;

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
    bIsInteraction = false;
    GetWorld()->GetTimerManager().ClearTimer(CountdownTimerHandle);

    if (TimerWidgetInstance)
    {
        TimerWidgetInstance->RemoveFromParent();
        TimerWidgetInstance = nullptr;
    }

    // 생성된 런포인트 액터들 삭제
    for (AActor* RunActor : SpawnedRunPoints)
    {
        if (RunActor && !RunActor->IsActorBeingDestroyed())
        {
            RunActor->Destroy();
        }
    }
    SpawnedRunPoints.Empty();

    if (cachedPlayer)
    {
        cachedPlayer->bIsInteractionEnd = false;
        cachedPlayer->bIsInteraction = false;
        cachedPlayer->CurrentInteractTarget = nullptr;
    }
}