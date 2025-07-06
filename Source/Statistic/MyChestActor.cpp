// Fill out your copyright notice in the Description page of Project Settings.


#include "MyChestActor.h"
#include "Components/SceneComponent.h"
#include "Components/BoxComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Blueprint/UserWidget.h"
#include "PlayerCharacter.h"
#include "GameFramework/PlayerController.h"
#include "NiagaraComponent.h"
#include "MyItemDropActor.h"
#include "Kismet/GameplayStatics.h"


// Sets default values
AMyChestActor::AMyChestActor()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

    // 1. SceneRoot 생성 및 루트 지정
    SceneRoot = CreateDefaultSubobject<USceneComponent>(TEXT("SceneRoot"));
    RootComponent = SceneRoot;

    // 2. 박스 콜리전 생성, SceneRoot에 붙임
    ChestCollision = CreateDefaultSubobject<UBoxComponent>(TEXT("ChestCollision"));
    ChestCollision->SetupAttachment(SceneRoot);
    ChestCollision->InitBoxExtent(FVector(50.f, 50.f, 40.f));
    ChestCollision->SetCollisionProfileName(TEXT("OverlapAllDynamic"));

    // 3. 스켈레탈 메시 생성, SceneRoot에 붙임
    ChestMesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("ChestMesh"));
    ChestMesh->SetupAttachment(SceneRoot);
    ChestMesh->SetRelativeLocation(FVector(0.f, 0.f, -40.f)); // 메시 크기에 맞게 조정

    OpenEffectComponent = CreateDefaultSubobject<UNiagaraComponent>(TEXT("OpenEffectComponent"));
    OpenEffectComponent->SetupAttachment(SceneRoot);
    OpenEffectComponent->bAutoActivate = false; // 상자 열릴 때만 활성화

    ItemEffectComponent = CreateDefaultSubobject<UNiagaraComponent>(TEXT("ItemEffectComponent"));
    ItemEffectComponent->SetupAttachment(SceneRoot);
    ItemEffectComponent->bAutoActivate = false;

    // 4. 애니메이션 시퀀스 초기화
    OpenAnimSequence = nullptr;
}

// Called when the game starts or when spawned
void AMyChestActor::BeginPlay()
{
    Super::BeginPlay();
    ChestCollision->OnComponentBeginOverlap.AddDynamic(this, &AMyChestActor::OnBeginOverlapCollision);
    ChestCollision->OnComponentEndOverlap.AddDynamic(this, &AMyChestActor::OnEndOverlapCollision);

    if (ItemEffectComponent)
    {
        ItemEffectComponent->OnSystemFinished.AddDynamic(this, &AMyChestActor::OnItemEffectFinished);
    }
}

// Called every frame
void AMyChestActor::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

    if (bIsShrinking)
    {
        ShrinkElapsed += DeltaTime;
        float Alpha = FMath::Clamp(ShrinkElapsed / ShrinkDuration, 0.f, 1.f);
        float Scale = FMath::Lerp(1.f, 0.f, Alpha);
        SetActorScale3D(FVector(Scale));

        if (Alpha >= 1.f)
        {
            bIsShrinking = false;
            Destroy();
        }
    }
}

void AMyChestActor::OnBeginOverlapCollision(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
    UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
    APlayerCharacter* Player = Cast<APlayerCharacter>(OtherActor);
    if (!Player || !Player->get_is_player()) return;

    // 이미 다른 플레이어가 상호작용 중이라면 무시
    if (cachedPlayer != nullptr && cachedPlayer != Player) return;

    APlayerController* Controller = Cast<APlayerController>(Player->GetController());
    if (!Controller) return;

    // 처음 접근한 플레이어 저장
    cachedPlayer = Player;
    cachedController = Controller;

    if (!interactionWidgetInstance && interactionWidgetClass)
    {
        interactionWidgetInstance = CreateWidget<UUserWidget>(Controller, interactionWidgetClass);
        if (interactionWidgetInstance)
        {
            interactionWidgetInstance->AddToViewport();
        }
    }

    Player->bIsInteraction = true;
    Player->CurrentInteractTarget = this;
}


// ...existing code...

void AMyChestActor::Interact(APlayerCharacter* InteractingPlayer)
{
    if (InteractingPlayer != cachedPlayer) return; // 내가 지정한 플레이어가 아닐 경우 무시

    if (interactionWidgetInstance)
    {
        interactionWidgetInstance->RemoveFromParent();
        interactionWidgetInstance = nullptr;
    }

    if (OpenEffectComponent && OpenEffectSystem)
    {
        OpenEffectComponent->SetAsset(OpenEffectSystem);
        OpenEffectComponent->Activate(true);
    }

    GetWorld()->GetTimerManager().SetTimer(
        ItemEffectTimerHandle, this, &AMyChestActor::PlayItemEffect, 2.0f, false
    );

    if (OpenAnimSequence)
    {
        ChestMesh->PlayAnimation(OpenAnimSequence, false);
    }

    InteractingPlayer->bIsInteraction = false;
    InteractingPlayer->CurrentInteractTarget = nullptr;

    // 플레이어 캐시 해제
    cachedPlayer = nullptr;
    cachedController = nullptr;
}

// 아이템 이펙트 실행 함수 추가
void AMyChestActor::PlayItemEffect()
{
    if (ItemEffectComponent)
    {
        if (ItemEffectSystem)
        {
            ItemEffectComponent->SetAsset(ItemEffectSystem);
        }
        ItemEffectComponent->Activate(true);
    }
    if (DroppedItemActorClass)
    {
        for (int32 i = 0; i < 5; ++i)
        {
            FVector SpawnLocation = GetActorLocation() + FVector(0.f, 0.f, 200.f);
            FRotator SpawnRotation = FRotator::ZeroRotator;
            AMyItemDropActor* SpawnedItem = GetWorld()->SpawnActor<AMyItemDropActor>(DroppedItemActorClass, SpawnLocation, SpawnRotation);
            if (SpawnedItem)
            {
                SpawnedItem->SpawnItem(SpawnLocation);
            }
        }
    }
}

void AMyChestActor::OnEndOverlapCollision(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
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

    Player->bIsInteraction = false;
    Player->CurrentInteractTarget = nullptr;

    // 해제
    cachedPlayer = nullptr;
    cachedController = nullptr;
}


void AMyChestActor::OnItemEffectFinished(UNiagaraComponent* PSystem)
{
    bIsShrinking = true;
    ShrinkElapsed = 0.f;
}