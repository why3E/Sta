// Fill out your copyright notice in the Description page of Project Settings.
#include "MyFireBall.h"
#include "MyMagicStatue.h"
#include "MyEnemyBase.h"
#include "PlayerCharacter.h"
#include "Components/SphereComponent.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "NiagaraComponent.h"
#include "NiagaraFunctionLibrary.h"
#include "ReceiveDamageInterface.h"
#include "Enums.h"
#include "MixTonadoInterface.h"
#include "BombAttackInterface.h"
#include "SESSION.h"

// Sets default values
AMyFireBall::AMyFireBall()
{
    SetElement(EClassType::CT_Fire);
    SetType(SKILL_FIRE_BALL);

    // 콜리전 컴포넌트 초기화
    CollisionComponent = CreateDefaultSubobject<USphereComponent>(TEXT("CollisionComponent"));
    CollisionComponent->SetSphereRadius(30.0f);
    CollisionComponent->SetCollisionProfileName(TEXT("Projectile")); // 콜리전 프로파일 설정
    RootComponent = CollisionComponent;

    // 나이아가라 파티클 컴포넌트 초기화
    FireBallNiagaraComponent = CreateDefaultSubobject<UNiagaraComponent>(TEXT("FireBallNiagaraComponent"));
    FireBallNiagaraComponent->SetupAttachment(CollisionComponent);
	FireBallNiagaraComponent->SetVisibility(true);

    // Projectile Movement 컴포넌트 초기화
    MovementComponent = CreateDefaultSubobject<UProjectileMovementComponent>(TEXT("MovementComponent"));
    MovementComponent->bRotationFollowsVelocity = true;
    MovementComponent->InitialSpeed = Speed; // 초기 속도 설정
    MovementComponent->MaxSpeed = Speed;     // 최대 속도 설정
    MovementComponent->bShouldBounce = false; // 바운스 여부 설정

    MovementComponent->ProjectileGravityScale = 0.0f;

    // 초기 상태 설정
    bIsHit = false;
}

// Called when the game starts or when spawned
void AMyFireBall::BeginPlay()
{
	Super::BeginPlay();
	MovementComponent->SetActive(false);
	FireBallNiagaraComponent->Activate(); // 나이아가라 효과 활성화
}

// Called every frame
void AMyFireBall::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

void AMyFireBall::PostInitializeComponents()
{
    Super::PostInitializeComponents();

    // Event Mapping
    CollisionComponent->OnComponentBeginOverlap.AddDynamic(this, &AMyFireBall::OnBeginOverlap);
}

void AMyFireBall::Fire(FVector TargetLocation)
{
    FVector LaunchDirection;
	
    // 방향 계산
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

    // 방향 지정 및 Projectile Movement Component 활성화
    MovementComponent->Velocity = LaunchDirection * MovementComponent->InitialSpeed;
    MovementComponent->Activate();

    // 3초 후 자동 삭제
    SetLifeSpan(3.0f);
}

void AMyFireBall::OnBeginOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
    if (!g_is_host || bIsHit || (Owner == OtherActor)) { return; }

    // TODO: 데미지 전달 로직 추가
    UE_LOG(LogTemp, Warning, TEXT("Hit Actor: %s"), *OtherActor->GetName());
    UE_LOG(LogTemp, Warning, TEXT("Hit Component: %s"), *OverlappedComp->GetName());
    
    if (OtherActor->IsA(AMySkillBase::StaticClass())) {
        // Skill - Skill Collision
        AMySkillBase* ptr = Cast<AMySkillBase>(OtherActor);

        if (g_c_skills.count(ptr->m_id)) {
            if (m_id < ptr->m_id) {
                bIsHit = true;

                {
                    CollisionEvent collision_event = SkillSkillEvent(m_id, ptr->GetType());
                    std::lock_guard<std::mutex> lock(g_s_collision_events_l);
                    g_s_collision_events.push(collision_event);

                    collision_event = SkillSkillEvent(ptr->GetId(), GetType());
                    g_s_collision_events.push(collision_event);
                }
            }
        }
    } else if (OtherActor->IsA(AMyEnemyBase::StaticClass())) {
        // Skill - Monster Collision
        AMyEnemyBase* ptr = Cast<AMyEnemyBase>(OtherActor);

        if (g_c_monsters.count(ptr->get_id())) {
            if (ptr->GetHP() > 0.0f) {
                bIsHit = true;

                {
                    CollisionEvent collision_event = SkillMonsterEvent(m_id);
                    std::lock_guard<std::mutex> lock(g_s_collision_events_l);
                    g_s_collision_events.push(collision_event);

                    collision_event = MonsterSkillEvent(ptr->get_id(), GetType(), GetActorLocation());
                    g_s_collision_events.push(collision_event);
                }
            }
        }
    } else if (OtherActor->IsA(APlayerCharacter::StaticClass())) {
        // Skill - Player Collision
        APlayerCharacter* ptr = Cast<APlayerCharacter>(OtherActor);

        if (g_c_players[ptr->get_id()]) {
            bIsHit = true;

            {
                CollisionEvent collision_event = SkillPlayerEvent(m_id, ptr->get_id());
                std::lock_guard<std::mutex> lock(g_s_collision_events_l);
                g_s_collision_events.push(collision_event);
            }
        }
    } else if (OtherActor->IsA(AMyMagicStatue::StaticClass())) {
        // Skill - Object Collision
        bIsHit = true;

        {
            CollisionEvent collision_event = SkillObjectEvent(m_id);
            std::lock_guard<std::mutex> lock(g_s_collision_events_l);
            g_s_collision_events.push(collision_event);
        }
    }
}

void AMyFireBall::Overlap(char skill_type) {
    if (FireBallHitShootSound) {
        UGameplayStatics::PlaySoundAtLocation(this, FireBallHitShootSound, GetActorLocation(),5.0f);
    }   

    // 나이아가라 파티클 시스템 비활성화
    if (FireBallNiagaraComponent) {
        FireBallNiagaraComponent->Deactivate();
    }

    // 히트 효과 생성
    if (HitEffectNiagaraSystem) {
        UNiagaraFunctionLibrary::SpawnSystemAtLocation(GetWorld(), HitEffectNiagaraSystem, GetActorLocation());
    }

    // 발사체 제거
    Destroy();
}

void AMyFireBall::Overlap(unsigned short object_id, bool collision_start) {
    if (FireBallHitShootSound) {
        UGameplayStatics::PlaySoundAtLocation(this, FireBallHitShootSound, GetActorLocation(), 5.0f);
    }

    // 나이아가라 파티클 시스템 비활성화
    if (FireBallNiagaraComponent) {
        FireBallNiagaraComponent->Deactivate();
    }

    // 히트 효과 생성
    if (HitEffectNiagaraSystem) {
        UNiagaraFunctionLibrary::SpawnSystemAtLocation(GetWorld(), HitEffectNiagaraSystem, GetActorLocation());
    }

    // 충돌 상태 설정
    bIsHit = true;

    // 발사체 제거
    Destroy();
}

void AMyFireBall::ActivateNiagara()
{
    if (FireBallNiagaraComponent)
    {
        FireBallNiagaraComponent->Activate();
        UE_LOG(LogTemp, Warning, TEXT("FireBall Niagara Component Activated"));
    }
    else
    {
        UE_LOG(LogTemp, Error, TEXT("FireBall Niagara Component is null!"));
    }
}