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

    g_c_objects[ID] = this;
}

void AMyBoomsGimmickTrigger::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
}

void AMyBoomsGimmickTrigger::Interact(APlayerCharacter* InteractingPlayer)
{
    if (bIsInteraction) return;

    Super::Interact(InteractingPlayer);

    gimmick_start_packet p;
    p.packet_size = sizeof(gimmick_start_packet);
    p.packet_type = C2H_GIMMICK_START_PACKET;
    p.object_id = ID;
    InteractingPlayer->do_send(&p);
}

void AMyBoomsGimmickTrigger::Active() {
    if (interactionWidgetInstance)
    {
        interactionWidgetInstance->RemoveFromParent();
        interactionWidgetInstance = nullptr;
    }

    if (!BombClass || BombSpawnTargets.Num() == 0) return;

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

                Bomb->set_id(g_c_object_id++);
                g_c_objects[Bomb->get_id()] = Bomb;
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

void AMyBoomsGimmickTrigger::End(bool succeed) {
    if (succeed) {
        EndTriggerSuccess();
    } else {
        EndTriggerFailed();
    }
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

void AMyBoomsGimmickTrigger::NotifyBombDestroyed()
{
    if (g_is_host) {
        if (bIsTriggerEnded) { return; }

        DestroyedBombs++;

        if (DestroyedBombs >= TotalBombs) {
            gimmick_end_packet p;
            p.packet_size = sizeof(gimmick_end_packet);
            p.packet_type = C2H_GIMMICK_END_PACKET;
            p.object_id = ID;
            p.succeed = true;
            cachedPlayer->do_send(&p);
        }
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
