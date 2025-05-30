#include "MyIceArrow.h"
#include "EnemyCharacter.h"
#include "PlayerCharacter.h"
#include "Components/SphereComponent.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "NiagaraComponent.h"
#include "NiagaraFunctionLibrary.h"
#include "ReceiveDamageInterface.h"
#include "Enums.h"
#include "SESSION.h"

// Sets default values
AMyIceArrow::AMyIceArrow()
{
    SetElement(EClassType::CT_Ice);

    // 콜리전 컴포넌트 초기화
    CollisionComponent = CreateDefaultSubobject<USphereComponent>(TEXT("CollisionComponent"));
    CollisionComponent->SetSphereRadius(30.0f);
    CollisionComponent->SetCollisionProfileName(TEXT("Projectile"));
    RootComponent = CollisionComponent;

    // 나이아가라 파티클 컴포넌트 초기화
    IceArrowNiagaraComponent = CreateDefaultSubobject<UNiagaraComponent>(TEXT("IceArrowNiagaraComponent"));
    IceArrowNiagaraComponent->SetupAttachment(CollisionComponent);
    IceArrowNiagaraComponent->SetVisibility(true);

    // Projectile Movement 컴포넌트 초기화
    MovementComponent = CreateDefaultSubobject<UProjectileMovementComponent>(TEXT("MovementComponent"));
    MovementComponent->bRotationFollowsVelocity = true;
    MovementComponent->InitialSpeed = Speed;
    MovementComponent->MaxSpeed = Speed;
    MovementComponent->bShouldBounce = false;
    MovementComponent->ProjectileGravityScale = 0.0f;

    // 초기 상태 설정
    bIsHit = false;
}

void AMyIceArrow::BeginPlay()
{
    Super::BeginPlay();
    MovementComponent->SetActive(false);
    IceArrowNiagaraComponent->Activate();
}

void AMyIceArrow::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
}

void AMyIceArrow::PostInitializeComponents()
{
    Super::PostInitializeComponents();

    if (CollisionComponent)
    {
        CollisionComponent->OnComponentBeginOverlap.AddDynamic(this, &AMyIceArrow::OnBeginOverlap);
    }
}

void AMyIceArrow::Fire(FVector TargetLocation)
{
    FVector LaunchDirection;

    if (Owner) {
        if ((TargetLocation - Owner->GetActorLocation()).Length() < 300.0f)
        {
            LaunchDirection = (TargetLocation - GetActorLocation()).GetSafeNormal();
            LaunchDirection.Z = 0.0f;
        }
        else
        {
            LaunchDirection = (TargetLocation - GetActorLocation()).GetSafeNormal();
        }
    } else {
        return;
    }

    MovementComponent->Velocity = LaunchDirection * MovementComponent->InitialSpeed;
    MovementComponent->Activate();

    SetLifeSpan(3.0f);
}

void AMyIceArrow::OnBeginOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
    if (!g_is_host || bIsHit) { return; } 

    if (OtherActor->IsA(AMySkillBase::StaticClass())) {
        // Skill - Skill Collision
        AMySkillBase* ptr = Cast<AMySkillBase>(OtherActor);

        if (g_c_skills.count(ptr->m_id)) {
            if (m_id < ptr->m_id) {
                bIsHit = true;

                collision_packet p;
                p.packet_size = sizeof(collision_packet);
                p.packet_type = C2H_COLLISION_PACKET;
                p.collision_type = SKILL_SKILL_COLLISION;
                p.attacker_id = m_id;
                p.victim_id = ptr->m_id;

                Cast<APlayerCharacter>(Owner)->do_send(&p);
            }
        }
    } else if (OtherActor->IsA(AEnemyCharacter::StaticClass())) {
        // Skill - Monster Collision
        AEnemyCharacter* ptr = Cast<AEnemyCharacter>(OtherActor);

        if (g_c_monsters.count(ptr->get_id())) {
            if (ptr->get_hp() > 0.0f) {
                bIsHit = true;

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
}

void AMyIceArrow::Overlap(AActor* OtherActor) {
    if (HitEffectNiagaraSystem) {
        UNiagaraFunctionLibrary::SpawnSystemAtLocation(GetWorld(), HitEffectNiagaraSystem, GetActorLocation());
    }

    // 발사체 제거
    Destroy();
}

void AMyIceArrow::Overlap(ACharacter* OtherActor) {
    // 충돌 상태 설정
    bIsHit = true;

    if (HitEffectNiagaraSystem) {
        UNiagaraFunctionLibrary::SpawnSystemAtLocation(GetWorld(), HitEffectNiagaraSystem, GetActorLocation());
    }

    // 발사체 제거
    Destroy();
}

void AMyIceArrow::ActivateNiagara()
{
    if (IceArrowNiagaraComponent)
    {
        IceArrowNiagaraComponent->Activate();
        UE_LOG(LogTemp, Warning, TEXT("IceArrow Niagara Component Activated"));
    }
    else
    {
        UE_LOG(LogTemp, Error, TEXT("IceArrow Niagara Component is null!"));
    }
}