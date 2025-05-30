#include "MyWindSkill.h"
#include "MyFireBall.h"
#include "MyFireSkill.h"
#include "MyIceArrow.h"
#include "MyIceSkill.h"
#include "EnemyCharacter.h"
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
    if (!g_is_host) return;

    FVector TornadoCenter = GetActorLocation();
    float PullSpeed = 500.f;        // 중심으로 당기는 속도
    float LiftSpeed = 800.f;         // 위로 올라가는 속도
    float SpinSpeed = 800.f;         // 회전 속도
    float AcceptanceRadius = 50.f;   // 너무 가까우면 안 당김

    for (APlayerCharacter* Player : OverlappingCharacters)
    {
        if (!IsValid(Player)) continue;

        FVector PlayerLocation = Player->GetActorLocation();
        FVector ToCenter = TornadoCenter - PlayerLocation;
        float Distance = ToCenter.Size();

        if (Distance < AcceptanceRadius) continue;

        ToCenter.Normalize();

        // 회전 벡터
        FVector RotateVector = FVector::CrossProduct(ToCenter, FVector::UpVector).GetSafeNormal();

        // 이동 방향 조합 (끌림 + 회전 + 상승)
        FVector MoveDir = ToCenter * PullSpeed + RotateVector * SpinSpeed + FVector(0, 0, LiftSpeed);
        FVector NewLocation = PlayerLocation + MoveDir * DeltaTime;

        // 강제로 위치 이동 (또는 Smooth하게 하고 싶으면 InterpTo 사용)
        Player->SetActorLocation(NewLocation, true);
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
    if (!g_is_host || !bIsValid) return;

    if (OtherActor && OtherActor != this) {
        if (OtherActor->IsA(AMySkillBase::StaticClass())) {
            // Skill - Skill Collision
            AMySkillBase* ptr = Cast<AMySkillBase>(OtherActor);
 
            if (g_c_skills.count(ptr->m_id)) {
                if (m_id < ptr->m_id) {
                    collision_packet p;
                    p.packet_size = sizeof(collision_packet);
                    p.packet_type = C2H_COLLISION_PACKET;
                    p.collision_type = SKILL_SKILL_COLLISION;
                    p.attacker_id = m_id;
                    p.victim_id = ptr->m_id;

                    Cast<APlayerCharacter>(Owner)->do_send(&p);
                }
            }
        }

        if (OtherActor->IsA(AEnemyCharacter::StaticClass())) {
            // Skill - Monster Collision
            AEnemyCharacter* ptr = Cast<AEnemyCharacter>(OtherActor);

            if (g_c_monsters.count(ptr->get_id())) {
                if (ptr->get_hp() > 0.0f) {
                    collision_packet p;
                    p.packet_size = sizeof(collision_packet);
                    p.packet_type = C2H_COLLISION_PACKET;
                    p.collision_type = SKILL_MONSTER_COLLISION;
                    p.attacker_id = m_id;
                    p.victim_id = ptr->get_id();

                    Cast<APlayerCharacter>(Owner)->do_send(&p);
                }
            }
        }

        if (OtherActor->IsA(APlayerCharacter::StaticClass())) {
            // Skill - Player Collision
            APlayerCharacter* ptr = Cast<APlayerCharacter>(OtherActor);

            if (g_c_players[ptr->get_id()]) {
                collision_packet p;
                p.packet_size = sizeof(collision_packet);
                p.packet_type = C2H_COLLISION_PACKET;
                p.collision_type = SKILL_PLAYER_COLLISION;
                p.attacker_id = m_id;
                p.victim_id = ptr->get_id();

                Cast<APlayerCharacter>(Owner)->do_send(&p);
            }
        }
    }
}

void AMyWindSkill::OnEndOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex) {
    //if (OtherActor && OtherActor->IsA(APlayerCharacter::StaticClass()))
    //{
    //    APlayerCharacter* Player = Cast<APlayerCharacter>(OtherActor);
    //    if (OverlappingCharacters.Contains(Player))
    //    {
    //        OverlappingCharacters.Remove(Player);

    //        UCharacterMovementComponent* MoveComp = Player->GetCharacterMovement();
    //        if (MoveComp)
    //        {
    //            MoveComp->GravityScale = 1.0f;
    //            MoveComp->SetMovementMode(MOVE_Walking);
    //        }

    //        UE_LOG(LogTemp, Warning, TEXT("Player %s removed from tornado"), *Player->GetName());
    //    }
    //}
}

void AMyWindSkill::Overlap(AActor* OtherActor)
{
    if ((OtherActor && OtherActor->IsA(AMyFireBall::StaticClass())) ||
        (OtherActor && OtherActor->IsA(AMyFireSkill::StaticClass()))) {
        SkillMixWindTonado(EClassType::CT_Fire, m_id);
    } else if ((OtherActor && OtherActor->IsA(AMyIceArrow::StaticClass())) ||
        (OtherActor && OtherActor->IsA(AMyIceSkill::StaticClass()))) {
        SkillMixWindTonado(EClassType::CT_Ice, m_id);
    } else if (OtherActor && OtherActor->IsA(AMyWindSkill::StaticClass())) {
        if (Cast<AMySkillBase>(OtherActor)->GetId() > m_id) {
            Destroy();
            return;
        }

        bIsValid = false;

        if (g_is_host) {
            FVector SpawnLocation = GetActorLocation();
            skill_create_packet p;
            p.packet_size = sizeof(skill_create_packet);
            p.packet_type = C2H_SKILL_CREATE_PACKET;
            p.skill_type = SKILL_WIND_WIND_TORNADO;
            p.old_skill_id = m_id;
            p.new_skill_x = SpawnLocation.X;
            p.new_skill_y = SpawnLocation.Y;
            p.new_skill_z = SpawnLocation.Z;

            Cast<APlayerCharacter>(Owner)->do_send(&p);
        }
    }
}

void AMyWindSkill::Overlap(ACharacter* OtherActor) {

}

void AMyWindSkill::CheckOverlappingActors()
{
}

void AMyWindSkill::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
    Super::EndPlay(EndPlayReason);
    GetWorld()->GetTimerManager().ClearTimer(CheckOverlapTimerHandle);

    for (APlayerCharacter* Player : OverlappingCharacters)
    {
        if (IsValid(Player))
        {
            UCharacterMovementComponent* MoveComp = Player->GetCharacterMovement();
            if (MoveComp)
            {
                MoveComp->GravityScale = 1.0f;
                MoveComp->SetMovementMode(MOVE_Walking);
            }
        }
    }
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

            if (g_c_collisions.count(skill_id))
            {
                while (!g_c_collisions[skill_id].empty())
                {
                    unsigned short other_id = g_c_collisions[skill_id].front();
                    g_c_collisions[skill_id].pop();

                    if (g_c_skills.count(other_id))
                    {
                        MixWindTonado->Overlap(g_c_skills[other_id]);
                        g_c_skills[other_id]->Overlap(g_c_skills[skill_id]);
                        UE_LOG(LogTemp, Error, TEXT("Skill %d and %d Collision Succeed!"), skill_id, other_id);
                    }
                }
            }
            UE_LOG(LogTemp, Warning, TEXT("MixWindTonado spawned at location: %s with ID : %d"), *SpawnLocation.ToString(), skill_id);
        }
    }

    Destroy();
}
