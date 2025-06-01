#include "MyMagicStatue.h"
#include "Components/BoxComponent.h"
#include "PlayerCharacter.h"
#include "Kismet/GameplayStatics.h"
#include "MyFadeWidget.h"
#include "MyUserSelectorUI.h"
#include "Blueprint/UserWidget.h"
#include "GameFramework/PlayerController.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "EngineUtils.h"

AMyMagicStatue::AMyMagicStatue()
{
    PrimaryActorTick.bCanEverTick = true;

    sceneComp = CreateDefaultSubobject<USceneComponent>(TEXT("SceneComp"));
    RootComponent = sceneComp;

    boxCollision = CreateDefaultSubobject<UBoxComponent>(TEXT("BoxCollision"));
    boxCollision->SetupAttachment(RootComponent);
}

void AMyMagicStatue::BeginPlay()
{
    Super::BeginPlay();

    boxCollision->OnComponentBeginOverlap.AddDynamic(this, &AMyMagicStatue::OnBeginOverlapCollision);
    boxCollision->OnComponentEndOverlap.AddDynamic(this, &AMyMagicStatue::OnEndOverlapCollision);

    UWorld* World = GetWorld();
    if (!World) return;

    int32 MinStatueNumber = INT32_MAX;
    NextStatue = nullptr;
    FirstStatue = nullptr;

    for (TActorIterator<AMyMagicStatue> It(World); It; ++It)
    {
        AMyMagicStatue* Statue = *It;

        if (!Statue) continue;

        // 가장 작은 번호 찾기
        if (Statue->StatueNumber < MinStatueNumber)
        {
            MinStatueNumber = Statue->StatueNumber;
            FirstStatue = Statue;
        }

        // 다음 번호 석상 찾기 (자기 번호 + 1)
        if (Statue != this && Statue->StatueNumber == StatueNumber + 1)
        {
            NextStatue = Statue;
        }
    }

    // 다음 번호 석상이 없으면 가장 작은 번호 석상으로
    if (!NextStatue)
    {
        NextStatue = FirstStatue;
    }
}

void AMyMagicStatue::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
}

void AMyMagicStatue::OnBeginOverlapCollision(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
    UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
    if (!OtherActor) return;

    APlayerCharacter* Player = Cast<APlayerCharacter>(OtherActor);

    if (!Player || !Player->get_is_player()) return;

    cachedPlayer = Player;
    cachedController = Cast<APlayerController>(Player->GetController());

    if (!cachedController || !interactionWidgetClass) return;

    if (!interactionWidgetInstance)
    {
        interactionWidgetInstance = CreateWidget<UUserWidget>(cachedController, interactionWidgetClass);
        if (interactionWidgetInstance)
        {
            interactionWidgetInstance->AddToViewport();
        }
    }

    Player->bIsInteraction = true;
    Player->CurrentInteractTarget = this;
}

void AMyMagicStatue::OnEndOverlapCollision(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
    UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
    APlayerCharacter* Player = Cast<APlayerCharacter>(OtherActor);

    if (!Player || !Player->get_is_player()) return;

    cachedController = Cast<APlayerController>(Player->GetController());

    if (!cachedController) return;

    if (interactionWidgetInstance)
    {
        interactionWidgetInstance->RemoveFromParent();
        interactionWidgetInstance = nullptr;
        Player->bIsInteraction = false;
        Player->CurrentInteractTarget = nullptr;
    }
}

void AMyMagicStatue::Interact(APlayerCharacter* InteractingPlayer)
{
    cachedPlayer = InteractingPlayer;
    cachedController = Cast<APlayerController>(InteractingPlayer->GetController());

    if (interactionWidgetInstance)
    {
        interactionWidgetInstance->RemoveFromParent();
        interactionWidgetInstance = nullptr;
    }

    if (!cachedController || !selectorWidget) return;

    if (cachedPlayer && cachedPlayer->GetCharacterMovement())
    {
        cachedPlayer->GetCharacterMovement()->DisableMovement();
    }

    selectorWidgetInstance = CreateWidget<UMyUserSelectorUI>(cachedController, selectorWidget);
    if (selectorWidgetInstance)
    {
        selectorWidgetInstance->AddToViewport();
        selectorWidgetInstance->StatueActor = this;

        selectorWidgetInstance->OnSelectorClosed.AddDynamic(this, &AMyMagicStatue::OnSelectorClosed);

        cachedController->bShowMouseCursor = true;

        FInputModeGameAndUI InputMode;
        InputMode.SetWidgetToFocus(selectorWidgetInstance->TakeWidget());
        InputMode.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);
        cachedController->SetInputMode(InputMode);
    }
}

void AMyMagicStatue::StartTeleportWithFade()
{
    if (!cachedPlayer || !cachedController || !fadeWidget) return;

    fadeWidgetInstance = CreateWidget<UMyFadeWidget>(cachedController, fadeWidget);
    if (fadeWidgetInstance)
    {
        fadeWidgetInstance->AddToViewport();

        FWidgetAnimationDynamicEvent OnFadeOutFinished;
        OnFadeOutFinished.BindDynamic(this, &AMyMagicStatue::HandleFadeInFinished);
        fadeWidgetInstance->BindToAnimationFinished(fadeWidgetInstance->FadeOut, OnFadeOutFinished);

        fadeWidgetInstance->PlayFadeOut();
    }
}

void AMyMagicStatue::HandleFadeInFinished()
{
    if (!cachedPlayer) return;

    PerformTeleport();

    if (fadeWidgetInstance && fadeWidgetInstance->FadeIn)
    {
        fadeWidgetInstance->PlayFadeIn();

        FWidgetAnimationDynamicEvent OnFadeInFinished;
        OnFadeInFinished.BindDynamic(this, &AMyMagicStatue::RemoveFadeWidget);
        fadeWidgetInstance->BindToAnimationFinished(fadeWidgetInstance->FadeIn, OnFadeInFinished);
    }
}

void AMyMagicStatue::RemoveFadeWidget()
{
    if (fadeWidgetInstance)
    {
        fadeWidgetInstance->RemoveFromParent();
        fadeWidgetInstance = nullptr;
    }

    if (cachedController)
    {
        cachedController->bShowMouseCursor = false;
        FInputModeGameOnly InputMode;
        cachedController->SetInputMode(InputMode);
    }

    if (cachedPlayer)
    {
        if (UCharacterMovementComponent* MoveComp = cachedPlayer->GetCharacterMovement())
        {
            MoveComp->SetMovementMode(MOVE_Walking);
        }
    }

    cachedPlayer = nullptr;
    cachedController = nullptr;

    if (selectorWidgetInstance)
    {
        selectorWidgetInstance->RemoveFromParent();
        selectorWidgetInstance = nullptr;
    }
}

void AMyMagicStatue::OnSelectorClosed()
{
    if (cachedPlayer)
    {
        if (UCharacterMovementComponent* MoveComp = cachedPlayer->GetCharacterMovement())
        {
            MoveComp->SetMovementMode(MOVE_Walking);
        }
    }

    if (cachedController)
    {
        cachedController->bShowMouseCursor = false;
        FInputModeGameOnly InputMode;
        cachedController->SetInputMode(InputMode);
    }

    if (selectorWidgetInstance)
    {
        selectorWidgetInstance->RemoveFromParent();
        selectorWidgetInstance = nullptr;
    }

    cachedPlayer = nullptr;
    cachedController = nullptr;
}

void AMyMagicStatue::PerformTeleport()
{
    if (!cachedPlayer || !NextStatue) return;

    FVector Forward = NextStatue->GetActorForwardVector();
    FVector TargetLocation = NextStatue->GetActorLocation() + Forward * 500.f;

    FVector TraceStart = TargetLocation + FVector(0, 0, 500.f);
    FVector TraceEnd = TargetLocation - FVector(0, 0, 500.f);
    FHitResult HitResult;
    FCollisionQueryParams Params;
    Params.AddIgnoredActor(cachedPlayer);

    UWorld* World = GetWorld();
    if (!World) return;

    float FinalZ = TargetLocation.Z;
    if (World->LineTraceSingleByChannel(HitResult, TraceStart, TraceEnd, ECC_Visibility, Params))
    {
        FinalZ = HitResult.ImpactPoint.Z;
    }

    TargetLocation.Z = FinalZ + 100.f;
    cachedPlayer->SetActorLocation(TargetLocation);

    player_teleport_packet p;
    p.packet_size = sizeof(player_teleport_packet);
    p.packet_type = C2H_PLAYER_TELEPORT_PACKET;
    p.id = cachedPlayer->get_id();
    p.x = TargetLocation.X; p.y = TargetLocation.Y; p.z = TargetLocation.Z;
    cachedPlayer->do_send(&p);

    UE_LOG(LogTemp, Warning, TEXT("Player teleported."));
}
