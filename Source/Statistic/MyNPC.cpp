// Fill out your copyright notice in the Description page of Project Settings.


#include "MyNPC.h"
#include "Components/SphereComponent.h"
#include "Components/WidgetComponent.h"
#include "PlayerCharacter.h"
#include "GameFramework/PlayerController.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "EngineUtils.h"
#include "MyNPCInteractionWidget.h"
#include "CineCameraActor.h"

AMyNPC::AMyNPC()
{
    // NPC의 상호작용 범위를 정의하는 Sphere Component 생성
    InteractionSphere = CreateDefaultSubobject<USphereComponent>(TEXT("InteractionSphere"));
    InteractionSphere->SetupAttachment(RootComponent);
    InteractionSphere->SetSphereRadius(200.0f); // 상호작용 범위 반경 설정

    // 상호작용 컴포넌트 생성
    WidgetInteractionComponent = CreateDefaultSubobject<UWidgetComponent>(TEXT("WidgetInteractionComponent"));
    WidgetInteractionComponent->SetupAttachment(RootComponent);
    WidgetInteractionComponent->SetVisibility(false);
    WidgetInteractionComponent->SetWidgetSpace(EWidgetSpace::Screen);
}


void AMyNPC::BeginPlay()
{
    Super::BeginPlay();
    WidgetInteractionComponent->SetVisibility(false);
    // 상호작용 범위가 겹치는 오브젝트에 대한 이벤트 바인딩
    InteractionSphere->OnComponentBeginOverlap.AddDynamic(this, &AMyNPC::OnBeginOverlap);
    InteractionSphere->OnComponentEndOverlap.AddDynamic(this, &AMyNPC::OnEndOverlap);

    for (TActorIterator<ACineCameraActor>it(GetWorld()); it; ++it)
	{
		ACineCameraActor* npcOwnCamera = *it;
        if (npcOwnCamera->Tags.Contains(npcCameraTagName))
        {
            npcInteractionCamera = npcOwnCamera;
        }
	}
}
void AMyNPC::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
    
    // NPC의 행동 로직을 여기에 추가할 수 있습니다.
}

void AMyNPC::OnBeginOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
    APlayerCharacter* Player = Cast<APlayerCharacter>(OtherActor);
    if (!Player || !Player->get_is_player()) return;

    // 이미 다른 플레이어가 상호작용 중이라면 무시
    if (cachedPlayer != nullptr && cachedPlayer != Player) return;

    APlayerController* PlayerController = Cast<APlayerController>(Player->GetController());
    if (!PlayerController) return;

    if (Player && PlayerController)
    {
        // 상호작용 위젯 표시
        WidgetInteractionComponent->SetVisibility(true);
    }

    Player->bIsInteraction = true;
    Player->CurrentInteractTarget = this;
}

void AMyNPC::OnEndOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
    APlayerCharacter* Player = Cast<APlayerCharacter>(OtherActor);
    if (!Player ) return;

    APlayerController* PlayerController = Cast<APlayerController>(Player->GetController());
    if (!PlayerController ) return;

    // 상호작용 위젯 숨김
    WidgetInteractionComponent->SetVisibility(false);

    Player->bIsInteraction = false;
    Player->CurrentInteractTarget = nullptr;
}

void AMyNPC::StartInteractionCamera()
{
    if (npcInteractionCamera)
    {
        cachedPlayer->GetCharacterMovement()->DisableMovement();
        cachedPlayer->GetMesh()->SetVisibility(false, true);
        cachedController->SetViewTargetWithBlend(npcInteractionCamera, 0.5f);
        cachedPlayer->HideUI();
        WidgetInteractionComponent->SetVisibility(false);
    }

    if (interactionWidgetClass)
    {
        if (!interactionWidgetInstance)
        {
            interactionWidgetInstance = CreateWidget<UMyNPCInteractionWidget>(cachedController, interactionWidgetClass);
         
            if (interactionWidgetInstance)
            {   
                interactionWidgetInstance->OutButtonClickEvent.AddDynamic(this, &AMyNPC::OutButtonClick);
                interactionWidgetInstance->AddToViewport();
            }
        }
        else
        {
            interactionWidgetInstance->SetVisibility(ESlateVisibility::Visible);
        }
    }
}

void AMyNPC::Interact(APlayerCharacter* InteractingPlayer)
{
    cachedPlayer = InteractingPlayer;
    cachedController = Cast<APlayerController>(InteractingPlayer->GetController());

    // 플레이어를 카메라 위치로 이동
    if (npcInteractionCamera && cachedPlayer)
    {
        FVector CamLocation = npcInteractionCamera->GetActorLocation();
        FRotator CamRotation = npcInteractionCamera->GetActorRotation();

        // NPC 위치를 바라보는 방향 계산
        FVector NPCDirection = GetActorLocation() - CamLocation;
        NPCDirection.Z = 0; // 평면상에서만 회전
        FRotator LookAtNPC = NPCDirection.Rotation();

        cachedPlayer->SetActorLocation(CamLocation);
        cachedPlayer->SetActorRotation(LookAtNPC);
    }

    cachedController->bShowMouseCursor = true;
    cachedPlayer->bIsInteractionWidgetOpen = true;

    StartInteractionCamera();
}

void AMyNPC::OutButtonClick()
{
    if (cachedPlayer)
    {
        cachedPlayer->GetCharacterMovement()->SetMovementMode(EMovementMode::MOVE_Walking);
        cachedPlayer->GetMesh()->SetVisibility(true, true);
        cachedController->SetViewTargetWithBlend(cachedPlayer, 0.5f);
        cachedPlayer->ShowUI();
        cachedController->bShowMouseCursor = false;
        FInputModeGameOnly InputMode;
        cachedController->SetInputMode(InputMode);
    }

    if (interactionWidgetInstance)
    {
        interactionWidgetInstance->SetVisibility(ESlateVisibility::Hidden);
    }
    cachedPlayer->bIsInteractionWidgetOpen = false;
}