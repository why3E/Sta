#include "MyMixWindTonado.h"
#include "MyFireBall.h"
#include "MyFireSkill.h"
#include "EnemyCharacter.h"
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

void AMyMixWindTonado::OnBeginOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult) {
    if (!g_is_host) return;

    if (OtherActor && OtherActor != this) {
        if (OtherActor->IsA(AMyMixWindTonado::StaticClass())) {
            return;
        }

        if (OtherActor->IsA(AMySkillBase::StaticClass())) {
            // Skill - Skill Collision
            AMySkillBase* ptr = Cast<AMySkillBase>(OtherActor);

            if (g_c_skills.count(ptr->m_id)) {
                if (m_id < ptr->m_id) {
                    {
                        CollisionEvent collision_event = SkillSkillEvent(m_id, ptr->GetType());
                        std::lock_guard<std::mutex> lock(g_s_monster_events_l);
                        g_s_collision_events.push(collision_event);

                        collision_event = SkillSkillEvent(ptr->GetId(), GetType());
                        g_s_collision_events.push(collision_event);
                    }
                }
            }
        } else if (OtherActor->IsA(AEnemyCharacter::StaticClass())) {
            // Skill - Monster Collision
            AEnemyCharacter* ptr = Cast<AEnemyCharacter>(OtherActor);

            if (g_c_monsters.count(ptr->get_id())) {
                if (ptr->get_hp() > 0.0f) {
                    {
                        CollisionEvent collision_event = MonsterSkillEvent(ptr->get_id(), GetType(), GetActorLocation());
                        std::lock_guard<std::mutex> lock(g_s_monster_events_l);
                        g_s_collision_events.push(collision_event);
                    }
                }
            }
        }

        UE_LOG(LogTemp, Warning, TEXT("Mix Tonado hit actor: %s"), *OtherActor->GetName());
    }
}

void AMyMixWindTonado::Overlap(char skill_type) {
    switch (skill_type) {
    case SKILL_FIRE_BALL:
    case SKILL_FIRE_WALL:
        SkillMixWindTonado(EClassType::CT_Fire, m_id);
        break;
    }
}

void AMyMixWindTonado::Overlap(unsigned short object_id, bool collision_start) {

}

void AMyMixWindTonado::CheckOverlappingActors() {

}

void AMyMixWindTonado::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
    Super::EndPlay(EndPlayReason);

    GetWorld()->GetTimerManager().ClearTimer(CheckOverlapTimerHandle);
}

void AMyMixWindTonado::SkillMixWindTonado(EClassType MixType, unsigned short skill_id)
{
    if (SkillElement != EClassType::CT_Wind) return;
    if (SkillElement == MixType) return;

    SkillElement = MixType;
    switch (MixType) {
    case EClassType::CT_Fire:
        if (FireEffect) {
            MixWindTonadoNiagaraComponent->SetAsset(FireEffect);
            MixWindTonadoNiagaraComponent->Activate(true); // 재실행
        }
        break;
    case EClassType::CT_Ice:
        if (IceEffect) {
            MixWindTonadoNiagaraComponent->SetAsset(IceEffect);
            MixWindTonadoNiagaraComponent->Activate(true); // 재실행
        }
        break;

    default:
        break;
    }

    UE_LOG(LogTemp, Warning, TEXT("Skill element set to: %d"), (int32)SkillElement);
}

