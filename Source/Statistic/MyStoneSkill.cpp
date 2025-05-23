#include "MyStoneSkill.h"
#include "EnemyCharacter.h"
#include "PlayerCharacter.h"
#include "Components/StaticMeshComponent.h"
#include "Components/SphereComponent.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "Kismet/GameplayStatics.h"
#include "NiagaraComponent.h"

// 생성자
AMyStoneSkill::AMyStoneSkill()
{
    PrimaryActorTick.bCanEverTick = true;

    // 콜리전
    CollisionComponent = CreateDefaultSubobject<USphereComponent>(TEXT("CollisionComponent"));
    RootComponent = CollisionComponent;
    CollisionComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision); // Fire 전까지 충돌 비활성화

    // 메시
    StoneMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("StoneMesh"));
    StoneMesh->SetupAttachment(RootComponent);
    StoneMesh->SetVisibility(false); // Fire 전까지 안보이게
    StoneMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision); // Fire 전까지 충돌 비활성화


    // 무브먼트
    ProjectileMovement = CreateDefaultSubobject<UProjectileMovementComponent>(TEXT("ProjectileMovement"));
    ProjectileMovement->bRotationFollowsVelocity = true;
    ProjectileMovement->ProjectileGravityScale = 1.0f;
    ProjectileMovement->InitialSpeed = Speed;
    ProjectileMovement->MaxSpeed = Speed;
    ProjectileMovement->bAutoActivate = false;

    // 나이아가라 이펙트
    TrailNiagaraComponent = CreateDefaultSubobject<UNiagaraComponent>(TEXT("TrailNiagaraComponent"));
    TrailNiagaraComponent->SetupAttachment(RootComponent);
    TrailNiagaraComponent->bAutoActivate = false;
}

// 발사 함수 (포물선)
void AMyStoneSkill::Fire(FVector FireLocation)
{
    // Fire 시점에 메시 보이게, 충돌 활성화
    if (StoneMesh)
    {
        StoneMesh->SetVisibility(true);
        StoneMesh->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
        StoneMesh->SetCollisionResponseToChannel(ECC_Camera, ECR_Ignore); // 카메라 충돌 무시
        if (Owner)
        {
            StoneMesh->IgnoreActorWhenMoving(Owner, true); // 오너와 충돌 무시
        }
    }

    if (CollisionComponent)
    {
        // QueryAndPhysics로 변경
        CollisionComponent->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
        CollisionComponent->SetCollisionResponseToChannel(ECC_Camera, ECR_Ignore); // 카메라 충돌 무시
        // 랜드스케이프 채널에 대한 응답 추가 (보통 WorldStatic)
        if (Owner)
        {
            CollisionComponent->IgnoreActorWhenMoving(Owner, true); // 오너와 충돌 무시
        }
    }

    FVector LaunchVelocity;
    FVector StartLocation = GetActorLocation() + FVector(0.0f, 0.0f, 100.0f); // 발사 시작 위치 (약간 위로)
    bool bHaveAimSolution = UGameplayStatics::SuggestProjectileVelocity(
        this,
        LaunchVelocity,
        StartLocation,
        FireLocation,
        ProjectileMovement->InitialSpeed,
        false,          // bHighArc
        0.0f,          // CollisionRadius
        0.0f,          // OverrideGravityZ
        ESuggestProjVelocityTraceOption::DoNotTrace
    );

    if (bHaveAimSolution)
    {
        ProjectileMovement->Velocity = LaunchVelocity;
        ProjectileMovement->Activate();

        if (TrailNiagaraComponent)
        {
            TrailNiagaraComponent->Activate(true);
        }

        UE_LOG(LogTemp, Log, TEXT("StoneSkill fired toward: %s"), *FireLocation.ToString());
    }
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("Failed to calculate trajectory to: %s"), *FireLocation.ToString());
    }
}

void AMyStoneSkill::PostInitializeComponents()
{
    Super::PostInitializeComponents();

    if (CollisionComponent)
    {
        CollisionComponent->OnComponentBeginOverlap.AddDynamic(this, &AMyStoneSkill::OnBeginOverlap);
    }

    // 예: 충돌 이벤트 바인딩 (나중에 필요 시 사용)
    // CollisionComponent->OnComponentHit.AddDynamic(this, &AMyStoneSkill::OnHit);
}

void AMyStoneSkill::BeginPlay()
{
    Super::BeginPlay();

    // 예: 시작 시 추가 로직
}

void AMyStoneSkill::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
    if (Owner && StoneMesh)
    {
        StoneMesh->IgnoreActorWhenMoving(Owner, true);
    }
    if (Owner && CollisionComponent)
    {
        CollisionComponent->IgnoreActorWhenMoving(Owner, true);
    }
    // 예: 날아가는 동안 회전 or 이펙트 제어
}

void AMyStoneSkill::Overlap(AActor* OtherActor)
{
    // 충돌 상태 설정
    bIsHit = true;

    // 발사체 제거
    Destroy();
}

void AMyStoneSkill::Overlap(ACharacter* OtherActor)
{
    // 충돌 상태 설정
    bIsHit = true;

    // 발사체 제거
    Destroy();
}

void AMyStoneSkill::OnBeginOverlap(class UPrimitiveComponent* OverlappedComp, class AActor* OtherActor, class UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
    if (!g_is_host || bIsHit || (Owner == OtherActor)) { return; } // 이미 충돌했거나 발사체의 소유자와 충돌한 경우 무시

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
    } else if (OtherActor->IsA(AEnemyCharacter::StaticClass())) {
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
    } else if (OtherActor->IsA(APlayerCharacter::StaticClass())) {
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