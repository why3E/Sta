// Fill out your copyright notice in the Description page of Project Settings.
#include "MyFireBall.h"
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
    // Tick 활성화 여부
    //PrimaryActorTick.bCanEverTick = true;

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
    if ((TargetLocation - Owner->GetActorLocation()).Length() < 300.0f)
    {
        LaunchDirection = (TargetLocation - GetActorLocation()).GetSafeNormal();
        LaunchDirection.Z = 0.0f;
    }
    else
    {
        LaunchDirection = (TargetLocation - GetActorLocation()).GetSafeNormal();
    }

    // 방향 지정 및 Projectile Movement Component 활성화
    MovementComponent->Velocity = LaunchDirection * MovementComponent->InitialSpeed;
    MovementComponent->Activate();

    // 3초 후 자동 삭제
    SetLifeSpan(3.0f);
}

void AMyFireBall::OnBeginOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
    if (!g_is_host || bIsHit || (Owner == OtherActor)) { return; } // 이미 충돌했거나 발사체의 소유자와 충돌한 경우 무시

    // TODO: 데미지 전달 로직 추가
    UE_LOG(LogTemp, Warning, TEXT("Hit Actor: %s"), *OtherActor->GetName());
    UE_LOG(LogTemp, Warning, TEXT("Hit Component: %s"), *OverlappedComp->GetName());
    
    // Skill - Skill Collision
    if (OtherActor->IsA(AMySkillBase::StaticClass())) {
        AMySkillBase* ptr = Cast<AMySkillBase>(OtherActor);

        if (g_skills.count(ptr->m_id)) {
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
}

void AMyFireBall::Overlap(AActor* OtherActor) {
    // 나이아가라 파티클 시스템 비활성화
    if (FireBallNiagaraComponent) {
        FireBallNiagaraComponent->Deactivate();
    }

    if (OtherActor && OtherActor->Implements<UReceiveDamageInterface>()) {
        // 데미지 전달
        FSkillInfo Info;
        Info.Damage = 10.f;
        Info.Element = EClassType::CT_Fire;
        Info.StunTime = 1.5f;
        Info.KnockbackDir = (OtherActor->GetActorLocation() - GetActorLocation()).GetSafeNormal();

        // 인터페이스로 캐스팅하여 함수 호출
        IReceiveDamageInterface* DamageReceiver = Cast<IReceiveDamageInterface>(OtherActor);
        if (DamageReceiver) {
            DamageReceiver->ReceiveSkillHit(Info, this);
            UE_LOG(LogTemp, Warning, TEXT("Skill hit applied to: %s"), *OtherActor->GetName());
        }
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