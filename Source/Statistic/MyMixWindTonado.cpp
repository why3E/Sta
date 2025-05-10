#include "MyMixWindTonado.h"
#include "PlayerCharacter.h"
#include "Components/StaticMeshComponent.h"
#include "NiagaraFunctionLibrary.h"
#include "Kismet/GameplayStatics.h"
#include "NiagaraSystem.h"
#include "ReceiveDamageInterface.h"
#include "SESSION.h"

AMyMixWindTonado::AMyMixWindTonado()
{
    PrimaryActorTick.bCanEverTick = true;

    CollisionMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("CollisionMesh"));
    RootComponent = CollisionMesh;

    static ConstructorHelpers::FObjectFinder<UStaticMesh> MeshAsset(TEXT("/Game/Toon_VFX_Vol1/Meshes/SM_VFX_BigTonado.SM_VFX_BigTonado"));
    if (MeshAsset.Succeeded())
    {
        CollisionMesh->SetStaticMesh(MeshAsset.Object);
        CollisionMesh->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
        CollisionMesh->SetCollisionProfileName(TEXT("OverlapAllDynamic"));
        CollisionMesh->SetGenerateOverlapEvents(true);
        CollisionMesh->SetVisibility(false);
        CollisionMesh->SetHiddenInGame(true);
    }

    MixWindTonadoNiagaraComponent = CreateDefaultSubobject<UNiagaraComponent>(TEXT("MixWindTonadoNiagaraComponent"));
    MixWindTonadoNiagaraComponent->SetupAttachment(CollisionMesh);
    MixWindTonadoNiagaraComponent->SetVisibility(true);
}

void AMyMixWindTonado::BeginPlay()
{
    Super::BeginPlay();

    GetWorld()->GetTimerManager().SetTimer(CheckOverlapTimerHandle, this, &AMyMixWindTonado::CheckOverlappingActors, 1.0f, true);
}

void AMyMixWindTonado::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
}

void AMyMixWindTonado::SpawnMixWindTondado(FVector ImpactPoint)
{
    SetActorLocation(ImpactPoint + FVector(0.0f, 0.0f, 150.0f)); // 높이 조정

    if (MixWindTonadoEffect)
    {
        MixWindTonadoNiagaraComponent->SetAsset(MixWindTonadoEffect);
        MixWindTonadoNiagaraComponent->SetVisibility(true);
        MixWindTonadoNiagaraComponent->Activate();
    }

    SetLifeSpan(WindTonadoDuration);
}

void AMyMixWindTonado::PostInitializeComponents()
{
    Super::PostInitializeComponents();
    CollisionMesh->OnComponentBeginOverlap.AddDynamic(this, &AMyMixWindTonado::OnBeginOverlap);
}

void AMyMixWindTonado::OnBeginOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
    UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
    if (!g_is_host) return;

    if (OtherActor && OtherActor != this)
    {
        if (OtherActor->IsA(AMyMixWindTonado::StaticClass()))
        {
            return;
        }

        UE_LOG(LogTemp, Warning, TEXT("Mix Tonado hit actor: %s"), *OtherActor->GetName());
    }
}

void AMyMixWindTonado::Overlap()
{
    // 확장용
}

void AMyMixWindTonado::CheckOverlappingActors()
{
    TArray<AActor*> CurrentOverlappingActors;
    CollisionMesh->GetOverlappingActors(CurrentOverlappingActors);

    for (AActor* OtherActor : CurrentOverlappingActors)
    {
        if (OtherActor && OtherActor->Implements<UReceiveDamageInterface>())
        {
            FSkillInfo Info;
            Info.Damage = 10.f;
            Info.Element = EClassType::CT_Wind;
            Info.StunTime = 1.5f;
            Info.KnockbackDir = (OtherActor->GetActorLocation() - GetActorLocation()).GetSafeNormal();

            IReceiveDamageInterface* DamageReceiver = Cast<IReceiveDamageInterface>(OtherActor);
            if (DamageReceiver)
            {
                DamageReceiver->ReceiveSkillHit(Info, this);
                UE_LOG(LogTemp, Warning, TEXT("Mix Skill hit applied to: %s"), *OtherActor->GetName());
            }
        }
    }
}

void AMyMixWindTonado::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
    Super::EndPlay(EndPlayReason);

    GetWorld()->GetTimerManager().ClearTimer(CheckOverlapTimerHandle);
}

void AMyMixWindTonado::SkillMixWindTonado(EClassType MixType)
{
    if(SkillElement != EClassType::CT_Wind) return;
    if(SkillElement == MixType) return;

    SkillElement = MixType;
    switch (MixType)
    {
    case EClassType::CT_Fire:
        if (FireEffect)
        {
            MixWindTonadoNiagaraComponent->SetAsset(FireEffect);
            MixWindTonadoNiagaraComponent->Activate(true); // 재실행
        }
        break;

    default:
        break;
    }

    UE_LOG(LogTemp, Warning, TEXT("Skill element set to: %d"), (int32)SkillElement);
}

