// Fill out your copyright notice in the Description page of Project Settings.


#include "MyGimmickTrigger.h"

#include "Components/BoxComponent.h"
#include "Components/SceneComponent.h"
#include "PlayerCharacter.h"
#include "Blueprint/UserWidget.h"
#include "GameFramework/PlayerController.h"
#include "Kismet/GameplayStatics.h"

// Sets default values
AMyGimmickTrigger::AMyGimmickTrigger()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	sceneComp = CreateDefaultSubobject<USceneComponent>(TEXT("SceneComp"));
    RootComponent = sceneComp;

    boxCollision = CreateDefaultSubobject<UBoxComponent>(TEXT("BoxCollision"));
    boxCollision->SetupAttachment(RootComponent);
}

// Called when the game starts or when spawned
void AMyGimmickTrigger::BeginPlay()
{
	Super::BeginPlay();
	boxCollision->OnComponentBeginOverlap.AddDynamic(this, &AMyGimmickTrigger::OnBeginOverlapCollision);
    boxCollision->OnComponentEndOverlap.AddDynamic(this, &AMyGimmickTrigger::OnEndOverlapCollision);
}

// Called every frame
void AMyGimmickTrigger::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

void AMyGimmickTrigger::Interact(APlayerCharacter* InteractingPlayer)
{
	cachedPlayer->bIsInteractionEnd = true;
    bIsInteraction = true;
}


void AMyGimmickTrigger::OnBeginOverlapCollision(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
    if (!OtherActor) return;

    APlayerCharacter* Player = Cast<APlayerCharacter>(OtherActor);
    if (!Player || !Player->get_is_player()) return;

    // 1. 다른 플레이어가 상호작용 중이면 무시
    if (cachedPlayer && cachedPlayer != Player && cachedPlayer->bIsInteraction) return;

    // 2. 플레이어 본인이 상호작용을 끝낸 상태면 무시
    if (Player->bIsInteractionEnd) return;

    // 3. 캐시
    cachedPlayer = Player;
    cachedController = Cast<APlayerController>(Player->GetController());
    if (!cachedController || !interactionWidgetClass) return;

    // 4. UI 위젯 생성
    if (!interactionWidgetInstance)
    {
        interactionWidgetInstance = CreateWidget<UUserWidget>(cachedController, interactionWidgetClass);
        if (interactionWidgetInstance)
        {
            interactionWidgetInstance->AddToViewport();
        }
    }

    // 5. 플레이어에게 상호작용 정보 설정
    Player->bIsInteraction = true;
    Player->CurrentInteractTarget = this;
}


void AMyGimmickTrigger::OnEndOverlapCollision(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
    APlayerCharacter* Player = Cast<APlayerCharacter>(OtherActor);
    if (!Player || Player != cachedPlayer) return;

    APlayerController* Controller = Cast<APlayerController>(Player->GetController());
    if (!Controller || Controller != cachedController) return;

    

    if (interactionWidgetInstance)
    {
        interactionWidgetInstance->RemoveFromParent();
        interactionWidgetInstance = nullptr;
    }

    if (Player->bIsInteractionEnd) return;

    Player->bIsInteraction = false;
    Player->CurrentInteractTarget = nullptr;

    // 초기화
    cachedPlayer = nullptr;
    cachedController = nullptr;
}
