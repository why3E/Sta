// Fill out your copyright notice in the Description page of Project Settings.


#include "MyItemWorldActor.h"
#include "Components/SphereComponent.h"
#include "Components/StaticMeshComponent.h"
#include "NiagaraComponent.h"
#include "NiagaraSystem.h"
#include "PlayerCharacter.h"
#include "Blueprint/UserWidget.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/PlayerController.h"


// Sets default values
AMyItemWorldActor::AMyItemWorldActor()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	ItemMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("ItemMesh"));
	RootComponent = ItemMesh;

	ItemMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	ItemMesh->SetGenerateOverlapEvents(false);

	ItemCollision = CreateDefaultSubobject<USphereComponent>(TEXT("ItemCollision"));
	ItemCollision->SetupAttachment(ItemMesh);
	ItemCollision->InitSphereRadius(40.f);

	ItemCollision->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	ItemCollision->SetCollisionResponseToAllChannels(ECR_Ignore);
	ItemCollision->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);
	ItemCollision->SetGenerateOverlapEvents(true);

	ItemEffectComponent = CreateDefaultSubobject<UNiagaraComponent>(TEXT("ItemEffectComponent"));
	ItemEffectComponent->SetupAttachment(ItemMesh);
	ItemEffectComponent->SetAutoActivate(true);
}

// Called when the game starts or when spawned
void AMyItemWorldActor::BeginPlay()
{
	Super::BeginPlay();
    ItemCollision->OnComponentBeginOverlap.AddDynamic(this, &AMyItemWorldActor::OnBeginOverlapCollision);
    ItemCollision->OnComponentEndOverlap.AddDynamic(this, &AMyItemWorldActor::OnEndOverlapCollision);

	if (ItemMeshes.Contains(ItemType) && ItemMeshes[ItemType])
    {
        ItemMesh->SetStaticMesh(ItemMeshes[ItemType]);
    }

}

// Called every frame
void AMyItemWorldActor::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

void AMyItemWorldActor::Interact(APlayerCharacter* InteractingPlayer)
{
	if (InteractingPlayer)
	{
		// 인벤토리 추가 등 아이템 획득 처리
		// InteractingPlayer->AddItemToInventory(this);
		InteractingPlayer->bIsInteraction = false;
		InteractingPlayer->CurrentInteractTarget = nullptr;
		Destroy();
	}
}

void AMyItemWorldActor::OnBeginOverlapCollision(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	APlayerCharacter* Player = Cast<APlayerCharacter>(OtherActor);
	if (!Player) return;

	APlayerController* cachedController = Cast<APlayerController>(Player->GetController());
	if (!cachedController) return;

	if (Player->bIsInteraction) return; // 이미 상호작용 중이면 무시

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

void AMyItemWorldActor::OnEndOverlapCollision(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
	APlayerCharacter* Player = Cast<APlayerCharacter>(OtherActor);
	if (!Player) return;

	APlayerController* cachedController = Cast<APlayerController>(Player->GetController());
	if (!cachedController) return;

	if (!Player || !cachedController) return;

	if (interactionWidgetInstance)
	{
		interactionWidgetInstance->RemoveFromParent();
		interactionWidgetInstance = nullptr;
		
		Player->bIsInteraction = false;
		Player->CurrentInteractTarget = nullptr;
	}
}