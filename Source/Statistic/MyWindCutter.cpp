// Fill out your copyright notice in the Description page of Project Settings.

#include "MyWindCutter.h"
#include "MyFireBall.h"
#include "EnemyCharacter.h"
#include "PlayerCharacter.h"
#include "Kismet/GameplayStatics.h"
#include "Components/BoxComponent.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "NiagaraComponent.h"
#include "NiagaraFunctionLibrary.h"
#include "NiagaraSystem.h"
#include "MyBombAttack.h"
#include "ReceiveDamageInterface.h"

#include "SESSION.h"

// Sets default values
AMyWindCutter::AMyWindCutter()
{
    SetElement(EClassType::CT_Wind);

    static ConstructorHelpers::FClassFinder<AMyBombAttack> BombBP(TEXT("/Game/Weapon/MyBombAttack.MyBombAttack_C"));
    if (BombBP.Succeeded())
    {
        BombAttackClass = BombBP.Class;
    }

	// 콜리전 컴포넌트 초기화
    CollisionComponent = CreateDefaultSubobject<UBoxComponent>(TEXT("CollisionComponent"));
    CollisionComponent->SetBoxExtent(FVector(40.0f, 120.0f, 5.0f)); // 박스 크기 설정
    CollisionComponent->SetCollisionProfileName(TEXT("Projectile")); // 콜리전 프로파일 설정
    RootComponent = CollisionComponent;

	// 나이아가라 파티클 컴포넌트 초기화
	WindCutterNiagaraComponent = CreateDefaultSubobject<UNiagaraComponent>(TEXT("WindCutterNiagaraComponent"));
	WindCutterNiagaraComponent->SetupAttachment(CollisionComponent);
	WindCutterNiagaraComponent->SetVisibility(true);

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

void AMyWindCutter::BeginPlay()
{
	Super::BeginPlay();
	MovementComponent->SetActive(false);
	WindCutterNiagaraComponent->Activate(); // 나이아가라 효과 활성화
}

// Called every frame
void AMyWindCutter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

void AMyWindCutter::PostInitializeComponents()
{
    Super::PostInitializeComponents();

    // Event Mapping
    CollisionComponent->OnComponentBeginOverlap.AddDynamic(this, &AMyWindCutter::OnBeginOverlap);
}

void AMyWindCutter::Fire(FVector TargetLocation)
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

void AMyWindCutter::OnBeginOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
    if (!g_is_host || bIsHit || (Owner == OtherActor)) { return; } // 이미 충돌했거나 발사체의 소유자와 충돌한 경우 무시
    
    if (OtherActor->IsA(AMySkillBase::StaticClass())) {
        // Skill - Skill Collision
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
    } else if (OtherActor->IsA(AEnemyCharacter::StaticClass())) {
        // Skill - Monster Collision
        AEnemyCharacter* ptr = Cast<AEnemyCharacter>(OtherActor);

        if (g_monsters.count(ptr->get_id())) {
            collision_packet p;
            p.packet_size = sizeof(collision_packet);
            p.packet_type = C2H_COLLISION_PACKET;
            p.collision_type = SKILL_MONSTER_COLLISION;
            p.attacker_id = m_id;
            p.victim_id = ptr->get_id();

            Cast<APlayerCharacter>(Owner)->do_send(&p);
            UE_LOG(LogTemp, Error, TEXT("[Client] Skill %d and Monster %d Collision"), p.attacker_id, p.victim_id);
        }
    }
}

void AMyWindCutter::Overlap(AActor* OtherActor) {
    // 나이아가라 파티클 시스템 비활성화
    if (WindCutterNiagaraComponent) {
        WindCutterNiagaraComponent->Deactivate();
    }

    if (OtherActor && OtherActor->IsA(AMyFireBall::StaticClass())) {
        // BombAttack
        FVector SpawnLocation = GetActorLocation();

        ch_skill_create_packet p;
        p.packet_size = sizeof(ch_skill_create_packet);
        p.packet_type = C2H_SKILL_CREATE_PACKET;
        p.skill_type = SKILL_WIND_FIRE_BOMB;
        p.old_skill_id = m_id;
        p.x = SpawnLocation.X; p.y = SpawnLocation.Y; p.z = SpawnLocation.Z;

        Cast<APlayerCharacter>(Owner)->do_send(&p);

        bIsHit = true;

        return;
    } 

    // 히트 효과 생성
    if (WindCutterNiagaraComponent) {
        UNiagaraFunctionLibrary::SpawnSystemAtLocation(GetWorld(), HitEffectNiagaraSystem, GetActorLocation());
    }
    
    // 충돌 상태 설정
    bIsHit = true;

    // 발사체 제거
    Destroy();
}

void AMyWindCutter::Overlap(ACharacter* OtherActor) {
    // 나이아가라 파티클 시스템 비활성화
    if (WindCutterNiagaraComponent) {
        WindCutterNiagaraComponent->Deactivate();
    }

    // 히트 효과 생성
    if (WindCutterNiagaraComponent) {
        UNiagaraFunctionLibrary::SpawnSystemAtLocation(GetWorld(), HitEffectNiagaraSystem, GetActorLocation());
    }

    // 충돌 상태 설정
    bIsHit = true;

    // 발사체 제거
    Destroy();
}

void AMyWindCutter::ActivateNiagara()
{
    if (WindCutterNiagaraComponent)
    {
		WindCutterNiagaraComponent->Activate();
        UE_LOG(LogTemp, Warning, TEXT("WindCutter Niagara Component Activated"));
    }
    else
    {
        UE_LOG(LogTemp, Error, TEXT("WindCutter Niagara Component is null!"));
    }
}

void AMyWindCutter::MixBombAttack(EClassType MixType, unsigned short skill_id)
{
    if (!BombAttackClass) {
        UE_LOG(LogTemp, Error, TEXT("BombAttackClass is not set!"));
        return;
    }

    // 현재 위치를 기준으로 스폰 위치 계산
    FVector SpawnLocation = GetActorLocation();
    FRotator SpawnRotation = GetActorRotation();

    // 지형 높이 확인
    FHitResult HitResult;
    FVector Start = SpawnLocation + FVector(0.0f, 0.0f, 500.0f); // 위에서 아래로 라인트레이스
    FVector End = SpawnLocation - FVector(0.0f, 0.0f, 500.0f);   // 아래로 500 유닛

    if (GetWorld()->LineTraceSingleByChannel(HitResult, Start, End, ECC_Visibility)) {
        // 지형의 충돌 지점 높이로 Z값 조정
        SpawnLocation.Z = HitResult.ImpactPoint.Z; // 바닥에서 50 유닛 위로 위치 조정
    }
    else {
        UE_LOG(LogTemp, Warning, TEXT("Line trace failed. Using default Z position."));
        SpawnLocation.Z += 50.0f; // 기본적으로 50 유닛 위로 조정
    }

    FTransform SpawnTransform(SpawnRotation, SpawnLocation);

    // BombAttack 생성
    AMyBombAttack* BombAttack = GetWorld()->SpawnActorDeferred<AMyBombAttack>(
        BombAttackClass,
        SpawnTransform,
        GetOwner(),
        nullptr,
        ESpawnActorCollisionHandlingMethod::AlwaysSpawn
    );

    if (BombAttack) {
        BombAttack->SetID(skill_id);
        BombAttack->SetOwner(GetOwner());
        BombAttack->SpawnBombAttack(SpawnLocation, MixType);

        g_skills[skill_id] = BombAttack;
        UGameplayStatics::FinishSpawningActor(BombAttack, SpawnTransform);

        if (g_collisions.count(skill_id)) {
            while (!g_collisions[skill_id].empty()) {
                unsigned short other_id = g_collisions[skill_id].front();
                g_collisions[skill_id].pop();

                if (g_skills.count(other_id)) {
                    BombAttack->Overlap(g_skills[other_id]);
                    g_skills[other_id]->Overlap(g_skills[skill_id]);
                    UE_LOG(LogTemp, Error, TEXT("Skill %d and %d Collision Succeed!"), skill_id, other_id);
                }
            }
        }

        UE_LOG(LogTemp, Warning, TEXT("BombAttack spawned at location: %s with MixType: %d"), *SpawnLocation.ToString(), static_cast<int32>(MixType));
    } else {
        UE_LOG(LogTemp, Error, TEXT("Failed to spawn BombAttack at location: %s"), *SpawnLocation.ToString());
    }

    Destroy();
}
