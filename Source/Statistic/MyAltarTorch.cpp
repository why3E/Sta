#include "MyAltarTorch.h"
#include "NiagaraComponent.h"
#include "Components/CapsuleComponent.h"
#include "MyAltarMain.h"
#include "MyFireSkill.h"
#include "MyFireBall.h"

// Sets default values
AMyAltarTorch::AMyAltarTorch()
{
    PrimaryActorTick.bCanEverTick = true;

    SceneRoot = CreateDefaultSubobject<USceneComponent>(TEXT("SceneRoot"));
    RootComponent = SceneRoot;

    TorchCollision = CreateDefaultSubobject<UCapsuleComponent>(TEXT("TorchCollision"));
    TorchCollision->InitCapsuleSize(30.f, 80.f);
    TorchCollision->SetupAttachment(SceneRoot);

    TorchEffect = CreateDefaultSubobject<UNiagaraComponent>(TEXT("TorchEffect"));
    TorchEffect->SetupAttachment(SceneRoot);
    TorchEffect->SetAutoActivate(false); // Initially deactivated
}

void AMyAltarTorch::BeginPlay()
{
    Super::BeginPlay();

    TorchCollision->OnComponentBeginOverlap.AddDynamic(this, &AMyAltarTorch::OnTorchBeginOverlap);
}

void AMyAltarTorch::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
}

void AMyAltarTorch::OnTorchBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
    UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
    UE_LOG(LogTemp, Warning, TEXT("Torch Overlap with: %s"), *OtherActor->GetName());

    if (bIsActivated) return;

    if (OtherActor &&
        (OtherActor->IsA(AMyFireSkill::StaticClass()) || OtherActor->IsA(AMyFireBall::StaticClass())))
    {
        UE_LOG(LogTemp, Warning, TEXT("Torch activated by fire skill or fire ball."));

        if (TorchEffect)
        {
            TorchEffect->Activate(true);
        }

        bIsActivated = true;

        if (AltarOwner)
		{
			AltarOwner->NotifyTorchActivated();
		}
		else
		{
			UE_LOG(LogTemp, Error, TEXT("[%s] AltarOwner is nullptr!"), *GetName());
		}
    }
}

void AMyAltarTorch::SetMainAltar(AMyAltarMain* InAltar)
{
	AltarOwner = InAltar;
	UE_LOG(LogTemp, Warning, TEXT("[%s] AltarOwner set: %s"), *GetName(), *GetNameSafe(AltarOwner));
}
