#include "MyRunPointActor.h"
#include "Components/SceneComponent.h"
#include "Components/SphereComponent.h"
#include "MyRunGimmickTrigger.h"
#include "PlayerCharacter.h"

// Sets default values
AMyRunPointActor::AMyRunPointActor()
{
    PrimaryActorTick.bCanEverTick = true;

    SceneComp = CreateDefaultSubobject<USceneComponent>(TEXT("SceneComp"));
    RootComponent = SceneComp;

    SphereComp = CreateDefaultSubobject<USphereComponent>(TEXT("SphereComp"));
    SphereComp->SetupAttachment(SceneComp);
    SphereComp->SetSphereRadius(100.f);
    SphereComp->SetCollisionProfileName(TEXT("OverlapAllDynamic"));
    SphereComp->SetGenerateOverlapEvents(true);
}

// Called when the game starts or when spawned
void AMyRunPointActor::BeginPlay()
{
    Super::BeginPlay();
    SphereComp->OnComponentBeginOverlap.AddDynamic(this, &AMyRunPointActor::OnBeginOverlap);
}

// Called every frame
void AMyRunPointActor::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
}

void AMyRunPointActor::Overlap() {
    if (g_c_objects.count(m_id)) {
        g_c_objects.erase(m_id);
    }

    Destroy();
}

void AMyRunPointActor::SetTriggerOwner(AMyRunGimmickTrigger* Trigger)
{
    TriggerOwner = Trigger;
}

void AMyRunPointActor::Destroyed()
{
    Super::Destroyed();

    if (TriggerOwner)
    {
        TriggerOwner->NotifyPointPassed();
    }
}

void AMyRunPointActor::OnBeginOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
    if (!g_is_host) { return; }

    if (OtherActor->IsA(APlayerCharacter::StaticClass()))
    {
        CollisionEvent collision_event = ObjectSkillEvent(m_id);
        std::lock_guard<std::mutex> lock(g_s_collision_events_l);
        g_s_collision_events.push(collision_event);
    }
}
