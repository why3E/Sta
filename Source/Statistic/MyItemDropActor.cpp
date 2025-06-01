#include "MyItemDropActor.h"
#include "NiagaraComponent.h"
#include "NiagaraSystem.h"
#include "Blueprint/UserWidget.h"
#include "Kismet/GameplayStatics.h"
#include "Components/SphereComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/World.h"
#include "GameFramework/PlayerController.h"

// Sets default values
AMyItemDropActor::AMyItemDropActor()
{
    PrimaryActorTick.bCanEverTick = true;

    // 메시 = 루트
    ItemMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("ItemMesh"));
    RootComponent = ItemMesh;
    ItemMesh->SetSimulatePhysics(true);
    ItemMesh->SetCollisionProfileName(TEXT("PhysicsActor"));

    // 콜리전: 메시(루트)에 붙임
    ItemCollision = CreateDefaultSubobject<USphereComponent>(TEXT("ItemCollision"));
    ItemCollision->InitSphereRadius(40.f);
    ItemCollision->SetupAttachment(ItemMesh);
    ItemCollision->SetCollisionProfileName(TEXT("OverlapAllDynamic"));
    ItemCollision->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
    ItemCollision->SetGenerateOverlapEvents(true);

    // 나이아가라: 메시(루트)에 붙임
    ItemEffectComponent = CreateDefaultSubobject<UNiagaraComponent>(TEXT("ItemEffectComponent"));
    ItemEffectComponent->SetupAttachment(ItemMesh);
    ItemEffectComponent->SetAutoActivate(false);
}

void AMyItemDropActor::BeginPlay()
{
    Super::BeginPlay();
    ItemCollision->OnComponentBeginOverlap.AddDynamic(this, &AMyItemDropActor::OnBeginOverlapCollision);
    ItemCollision->OnComponentEndOverlap.AddDynamic(this, &AMyItemDropActor::OnEndOverlapCollision);

}

void AMyItemDropActor::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    // 나이아가라 위치만 메시와 맞추고, 회전은 항상 0
    if (ItemEffectComponent && ItemMesh)
    {
        ItemEffectComponent->SetWorldLocation(ItemMesh->GetComponentLocation());
        ItemEffectComponent->SetWorldRotation(FRotator::ZeroRotator);
    }
}

void AMyItemDropActor::SpawnItem(const FVector& StartLocation)
{
    float RandX = FMath::FRandRange(-400.f, 400.f);
    float RandY = FMath::FRandRange(-400.f, 400.f);
    FVector End = StartLocation - FVector(RandX, RandY, 300.f);
    FHitResult Hit;
    FCollisionQueryParams Params;
    Params.bTraceComplex = true;


    bool bHit = GetWorld()->LineTraceSingleByChannel(Hit, StartLocation, End, ECC_Visibility, Params);

    FVector SpawnLocation = StartLocation;
    if (bHit)
    {
        SpawnLocation.Z = Hit.ImpactPoint.Z + 1.0f;
    }

    if (ItemMesh)
    {
        ItemMesh->SetVisibility(true);
        ItemMesh->SetWorldLocation(SpawnLocation);
        ItemMesh->SetWorldRotation(GetActorRotation());
        FVector UpwardImpulse = FVector(0.f, 0.f, 1000.f);
        ItemMesh->AddImpulse(UpwardImpulse, NAME_None, true);
    }

    if (ItemEffectSystem && ItemEffectComponent)
    {
        ItemEffectComponent->SetAsset(ItemEffectSystem);
        ItemEffectComponent->Activate(true);
    }
}

void AMyItemDropActor::Interact(APlayerCharacter* InteractingPlayer)
{
    if (interactionWidgetInstance)
    {
        interactionWidgetInstance->RemoveFromParent();
        interactionWidgetInstance = nullptr;
    }

    if (InteractingPlayer)
    {
        // 인벤토리 추가 등 아이템 획득 처리
        // InteractingPlayer->AddItemToInventory(this);
        Destroy();
    }
}

void AMyItemDropActor::OnBeginOverlapCollision(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
    UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
    APlayerCharacter* Player = Cast<APlayerCharacter>(OtherActor);
    if (!Player) return;

    APlayerController* cachedController = Cast<APlayerController>(Player->GetController());
    if (!cachedController) return;

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


void AMyItemDropActor::OnEndOverlapCollision(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
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
