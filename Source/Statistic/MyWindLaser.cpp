#include "MyWindLaser.h"
#include "NiagaraComponent.h"
#include "Components/CapsuleComponent.h"
#include "GameFramework/Actor.h"
#include "TimerManager.h"
#include "PlayerCharacter.h"

AMyWindLaser::AMyWindLaser()
{
    PrimaryActorTick.bCanEverTick = true;

    RootScene = CreateDefaultSubobject<USceneComponent>(TEXT("RootScene"));
    RootComponent = RootScene;

    ChargingEffect = CreateDefaultSubobject<UNiagaraComponent>(TEXT("ChargingEffect"));
    ChargingEffect->SetupAttachment(RootScene);
    ChargingEffect->SetAutoActivate(false);

    FiringEffect = CreateDefaultSubobject<UNiagaraComponent>(TEXT("FiringEffect"));
    FiringEffect->SetupAttachment(RootScene);
    FiringEffect->SetAutoActivate(false);

    CollisionCapsule = CreateDefaultSubobject<UCapsuleComponent>(TEXT("CollisionCapsule"));
    CollisionCapsule->SetupAttachment(RootScene);
    CollisionCapsule->SetCollisionEnabled(ECollisionEnabled::NoCollision);
    CollisionCapsule->SetGenerateOverlapEvents(false);

    bIsFiring = false;
}

void AMyWindLaser::PostInitializeComponents()
{
    Super::PostInitializeComponents();

    if (CollisionCapsule)
    {
        CollisionCapsule->OnComponentBeginOverlap.AddDynamic(this, &AMyWindLaser::OnOverlapBegin);
    }
}

void AMyWindLaser::BeginPlay()
{
    Super::BeginPlay();
}

void AMyWindLaser::SpawnChargingLaser()
{
    if (ChargingEffect)
    {
        ChargingEffect->OnSystemFinished.Clear();
        ChargingEffect->OnSystemFinished.AddDynamic(this, &AMyWindLaser::OnChargingFinished);
        ChargingEffect->Activate(true);

        UE_LOG(LogTemp, Log, TEXT("WindLaser: Charging started"));
    }
}

void AMyWindLaser::SpawnFiringLaser()
{
    bIsFiring = true;

    if (FiringEffect)
    {
        FiringEffect->OnSystemFinished.Clear();
        FiringEffect->OnSystemFinished.AddDynamic(this, &AMyWindLaser::OnFiringFinished);
        FiringEffect->Activate(true);

        UE_LOG(LogTemp, Log, TEXT("WindLaser: Firing effect activated"));
    }

    if (CollisionCapsule)
    {
        CollisionCapsule->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
        CollisionCapsule->SetGenerateOverlapEvents(true);

        UE_LOG(LogTemp, Log, TEXT("WindLaser: Collision enabled"));
    }
}

void AMyWindLaser::OnChargingFinished(UNiagaraComponent* PSystem)
{
    UE_LOG(LogTemp, Log, TEXT("WindLaser: Charging finished"));

    // 외부에서 SpawnFiringLaser() 호출 필요
}

void AMyWindLaser::OnFiringFinished(UNiagaraComponent* PSystem)
{
    bIsFiring = false;

    if (CollisionCapsule)
    {
        CollisionCapsule->SetCollisionEnabled(ECollisionEnabled::NoCollision);
        CollisionCapsule->SetGenerateOverlapEvents(false);

        UE_LOG(LogTemp, Log, TEXT("WindLaser: Firing finished, collision disabled"));
    }
    Destroy();
}

void AMyWindLaser::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
}

void AMyWindLaser::EndPlay(EEndPlayReason::Type EndPlayReason)
{
    Super::EndPlay(EndPlayReason);
}

void AMyWindLaser::OnOverlapBegin(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult) {
    if (!g_is_host || !bIsFiring || (Owner == OtherActor)) { return; }

    if (OtherActor || OtherActor != this) {
        if (OtherActor->IsA(AMySkillBase::StaticClass())) {
            // Skill - Skill Collision
            AMySkillBase* ptr = Cast<AMySkillBase>(OtherActor);

            if (g_c_skills.count(ptr->m_id)) {
                if (m_id < ptr->m_id) {
                    {
                        CollisionEvent collision_event = SkillSkillEvent(m_id, ptr->GetType());
                        std::lock_guard<std::mutex> lock(g_s_collision_events_l);
                        g_s_collision_events.push(collision_event);

                        collision_event = SkillSkillEvent(ptr->GetId(), GetType());
                        g_s_collision_events.push(collision_event);
                    }
                }
            }
        } else if (OtherActor->IsA(APlayerCharacter::StaticClass())) {
            // Skill - Player Collision
            APlayerCharacter* ptr = Cast<APlayerCharacter>(OtherActor);

            if (g_c_players[ptr->get_id()]) {
                {
                    CollisionEvent collision_event = SkillPlayerEvent(m_id, ptr->get_id());
                    std::lock_guard<std::mutex> lock(g_s_collision_events_l);
                    g_s_collision_events.push(collision_event);
                }
            }
        }

        UE_LOG(LogTemp, Warning, TEXT("WindLaser hit: %s"), *OtherActor->GetName());
    }
}
