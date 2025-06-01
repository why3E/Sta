#include "MyWindSkill.h"
#include "MyFireBall.h"
#include "MyFireSkill.h"
#include "MyIceArrow.h"
#include "MyIceSkill.h"
#include "MyEnemyBase.h"
#include "PlayerCharacter.h"
#include "Components/StaticMeshComponent.h"
#include "NiagaraFunctionLibrary.h"
#include "GameFramework/Actor.h"
#include "Kismet/GameplayStatics.h"
#include "NiagaraSystem.h"
#include "MyMixWindTonado.h"
#include "ReceiveDamageInterface.h"
#include "EngineUtils.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "SESSION.h"

// Sets default values
AMyWindSkill::AMyWindSkill()
{
    SetElement(EClassType::CT_Wind);
    SetType(SKILL_WIND_TORNADO);

    PrimaryActorTick.bCanEverTick = true;

    static ConstructorHelpers::FClassFinder<AMyMixWindTonado> MixBP(TEXT("/Game/Weapon/MyMixWindTonado.MyMixWindTonado_C"));
    if (MixBP.Succeeded())
    {
        MixWindTonadoClass = MixBP.Class;
    }

    CollisionMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("CollisionMesh"));
    RootComponent = CollisionMesh;

    static ConstructorHelpers::FObjectFinder<UStaticMesh> MeshAsset(TEXT("/Game/Toon_VFX_Vol1/Meshes/SM_VFX_WindTonado.SM_VFX_WindTonado"));
    if (MeshAsset.Succeeded())
    {
        CollisionMesh->SetStaticMesh(MeshAsset.Object);
        CollisionMesh->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
        CollisionMesh->SetCollisionProfileName(TEXT("OverlapAllDynamic"));
        CollisionMesh->SetGenerateOverlapEvents(true);
        CollisionMesh->SetVisibility(false);
        CollisionMesh->SetHiddenInGame(true);
    }

    WindTonadoNiagaraComponent = CreateDefaultSubobject<UNiagaraComponent>(TEXT("WindTonadoNiagaraComponent"));
    WindTonadoNiagaraComponent->SetupAttachment(CollisionMesh);
    WindTonadoNiagaraComponent->SetVisibility(true);
}

void AMyWindSkill::BeginPlay()
{
    Super::BeginPlay();
    GetWorld()->GetTimerManager().SetTimer(CheckOverlapTimerHandle, this, &AMyWindSkill::CheckOverlappingActors, 1.0f, true);
}

void AMyWindSkill::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    FVector TornadoCenter = GetActorLocation();

    float PullSpeed = 500.f;        // 중심으로 당기는 속도
    float LiftSpeed = 800.f;         // 위로 올라가는 속도
    float SpinSpeed = 800.f;         // 회전 속도
    float AcceptanceRadius = 50.f;   // 너무 가까우면 안 당김

    for (APlayerCharacter* Player : OverlappingCharacters) {
        if (IsValid(Player)) {
            FVector PlayerLocation = Player->GetActorLocation();
            FVector ToCenter = TornadoCenter - PlayerLocation;
            float Distance = ToCenter.Size();

            if (Distance < AcceptanceRadius) { continue; }

            ToCenter.Normalize();

            // 회전 벡터
            FVector RotateVector = FVector::CrossProduct(ToCenter, FVector::UpVector).GetSafeNormal();

            // 이동 방향 조합 (끌림 + 회전 + 상승)
            FVector MoveDir = ToCenter * PullSpeed + RotateVector * SpinSpeed + FVector(0, 0, LiftSpeed);
            FVector TargetLocation = PlayerLocation + MoveDir * DeltaTime;
            FVector NewLocation = FMath::VInterpTo(PlayerLocation, TargetLocation, DeltaTime, 5.f);

            // 강제로 위치 이동 (또는 Smooth하게 하고 싶으면 InterpTo 사용)
            Player->SetActorLocation(NewLocation, true);
            Player->LaunchCharacter(FVector(0, 0, 50), false, false);
        }
    }
}

void AMyWindSkill::SpawnWindTonado(FVector Location)
{
    SetActorLocation(Location);

    if (WindTonadoEffect)
    {
        WindTonadoNiagaraComponent->SetAsset(WindTonadoEffect);
        WindTonadoNiagaraComponent->SetVisibility(true);
        WindTonadoNiagaraComponent->Activate();
    }

    SetLifeSpan(WindTonadoDuration);
}

void AMyWindSkill::PostInitializeComponents()
{
    Super::PostInitializeComponents();
    CollisionMesh->OnComponentBeginOverlap.AddDynamic(this, &AMyWindSkill::OnBeginOverlap);
    CollisionMesh->OnComponentEndOverlap.AddDynamic(this, &AMyWindSkill::OnEndOverlap);
}

void AMyWindSkill::OnBeginOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult) {
    if (!g_is_host || !bIsValid) { return; }

    if (OtherActor && OtherActor != this) {
        if (OtherActor->IsA(AMySkillBase::StaticClass())) {
            // Skill - Skill Collision
            AMySkillBase* ptr = Cast<AMySkillBase>(OtherActor);
 
            if (g_c_skills.count(ptr->m_id)) {
                if (m_id < ptr->m_id) {
                    {
                        CollisionEvent collision_event = SkillSkillEvent(m_id, ptr->GetType());
                        std::lock_guard<std::mutex> lock(g_s_collision_events_l);
                        g_s_collision_events.push(collision_event);

                        if (SKILL_WIND_TORNADO == ptr->GetType()) {
                            collision_event = SkillSkillEvent(ptr->GetId(), INVALID_SKILL_ID);
                            g_s_collision_events.push(collision_event);
                        } else {
                            collision_event = SkillSkillEvent(ptr->GetId(), GetType());
                            g_s_collision_events.push(collision_event);
                        }
                    }
                }
            }
        }

        if (OtherActor->IsA(AMyEnemyBase::StaticClass())) {
            if (Owner != OtherActor) {
                // Skill - Monster Collision
                AMyEnemyBase* ptr = Cast<AMyEnemyBase>(OtherActor);

                if (g_c_monsters.count(ptr->get_id())) {
                    if (ptr->GetHP() > 0.0f) {
                        CollisionEvent collision_event = MonsterSkillEvent(ptr->get_id(), GetType(), GetActorLocation());
                        std::lock_guard<std::mutex> lock(g_s_collision_events_l);
                        g_s_collision_events.push(collision_event);
                    }
                }
            }
        }

        if (OtherActor->IsA(APlayerCharacter::StaticClass())) {
            // Skill - Player Collision
            APlayerCharacter* ptr = Cast<APlayerCharacter>(OtherActor);

            {
                CollisionEvent collision_event = SkillPlayerEvent(m_id, ptr->get_id());
                std::lock_guard<std::mutex> lock(g_s_collision_events_l);
                g_s_collision_events.push(collision_event);

                collision_event = PlayerSkillEvent(ptr->get_id(), GetType());
                g_s_collision_events.push(collision_event);
            }
        }
    }
}

void AMyWindSkill::OnEndOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex) {
    if (!g_is_host || !bIsValid) { return; }

    if (OtherActor && OtherActor->IsA(APlayerCharacter::StaticClass())) {
        APlayerCharacter* ptr = Cast<APlayerCharacter>(OtherActor);

        {
            CollisionEvent collision_event = SkillPlayerEvent(m_id, ptr->get_id());
            collision_event.collision_start = false;
            std::lock_guard<std::mutex> lock(g_s_collision_events_l);
            g_s_collision_events.push(collision_event);

            collision_event = PlayerSkillEvent(ptr->get_id(), GetType());
            collision_event.collision_start = false;
            g_s_collision_events.push(collision_event);
        }
    }
}

void AMyWindSkill::Overlap(char skill_type) {
    switch (skill_type) {
    case INVALID_SKILL_ID:
        Destroy();
        return;

    case SKILL_FIRE_BALL:
    case SKILL_FIRE_WALL:
        SkillMixWindTonado(EClassType::CT_Fire, m_id);
        break;

    case SKILL_ICE_ARROW:
    case SKILL_ICE_WALL:
        SkillMixWindTonado(EClassType::CT_Ice, m_id);
        break;

    case SKILL_WIND_TORNADO:
        bIsValid = false;

        if (g_is_host) {
            FVector SpawnLocation = GetActorLocation();

            Event event = SkillCreateEvent(m_id, SKILL_WIND_WIND_TORNADO, SpawnLocation);
            std::lock_guard<std::mutex> lock(g_s_events_l);
            g_s_events.push(event);
        }
        break;
    }
}

void AMyWindSkill::Overlap(unsigned short object_id, bool collision_start) {
    // Skill - Player Collision
    if ((0 <= object_id) && (object_id < MAX_CLIENTS)) {
        APlayerCharacter* ptr = Cast<APlayerCharacter>(g_c_players[object_id]);

        if (ptr != nullptr) {
            if (collision_start) {
                if (!OverlappingCharacters.Contains(ptr)) {
                    OverlappingCharacters.Add(ptr);
                }
            } else {
                if (OverlappingCharacters.Contains(ptr)) {
                    OverlappingCharacters.Remove(ptr);
                }
            }
        }
    }
}

void AMyWindSkill::CheckOverlappingActors() {

}

void AMyWindSkill::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
    Super::EndPlay(EndPlayReason);

    GetWorld()->GetTimerManager().ClearTimer(CheckOverlapTimerHandle);

    OverlappingCharacters.Empty();
}

void AMyWindSkill::SkillMixWindTonado(EClassType MixType, unsigned short skill_id)
{
    SetElement(MixType);

    switch (MixType)
    {
    case EClassType::CT_Fire:
        if (FireEffect)
        {
            WindTonadoNiagaraComponent->SetAsset(FireEffect);
            WindTonadoNiagaraComponent->Activate(true);
        }
        break;

    case EClassType::CT_Wind:
        SpawnMixTonado(skill_id);
        break;

    case EClassType::CT_Ice:
        if (IceEffect)
        {
            WindTonadoNiagaraComponent->SetAsset(IceEffect);
            WindTonadoNiagaraComponent->Activate(true);
        }
        break;

    default:
        break;
    }
}

void AMyWindSkill::SpawnMixTonado(unsigned short skill_id)
{
    FVector SpawnLocation = GetActorLocation();
    UE_LOG(LogTemp, Error, TEXT("MixTornado %d Spawning"), skill_id);

    if (MixWindTonadoClass)
    {
        FTransform SpawnTransform(FRotator::ZeroRotator, SpawnLocation);
        AMyMixWindTonado* MixWindTonado = GetWorld()->SpawnActorDeferred<AMyMixWindTonado>(
            MixWindTonadoClass, SpawnTransform, GetOwner(), nullptr, ESpawnActorCollisionHandlingMethod::AlwaysSpawn);

        if (MixWindTonado)
        {
            MixWindTonado->SetID(skill_id);
            MixWindTonado->SetOwner(GetOwner());
            MixWindTonado->SetActorLocation(SpawnLocation);

            g_c_skills.emplace(skill_id, MixWindTonado);
            UGameplayStatics::FinishSpawningActor(MixWindTonado, SpawnTransform);

            if (g_c_skill_collisions.count(skill_id)) {
                while (!g_c_skill_collisions[skill_id].empty()) {
                    char skill_type = g_c_skill_collisions[skill_id].front();
                    g_c_skill_collisions[skill_id].pop();

                    g_c_skills[skill_id]->Overlap(skill_type);
                }
            }

            if (g_c_object_collisions.count(skill_id)) {
                while (!g_c_object_collisions[skill_id].empty()) {
                    unsigned short object_id = g_c_object_collisions[skill_id].front();
                    g_c_object_collisions[skill_id].pop();

                    g_c_skills[skill_id]->Overlap(object_id);
                }
            }
        }
    }

    Destroy();
}
