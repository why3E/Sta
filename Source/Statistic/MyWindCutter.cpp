// Fill out your copyright notice in the Description page of Project Settings.

#include "MyWindCutter.h"
#include "Components/BoxComponent.h"
#include "PlayerCharacter.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "NiagaraComponent.h"
#include "NiagaraFunctionLibrary.h"
#include "NiagaraSystem.h"

#include "ReceiveDamageInterface.h"

#include "SESSION.h"

// Sets default values
AMyWindCutter::AMyWindCutter()
{
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

    // 충돌한 액터 로그 출력
    UE_LOG(LogTemp, Warning, TEXT("Hit Actor: %s"), *OtherActor->GetName());
    UE_LOG(LogTemp, Warning, TEXT("Hit Component: %s"), *OverlappedComp->GetName());
    
    for (const auto& [id, skill] : g_skills) {
        if (skill && (skill == OtherActor)) {
            collision_packet p;
            p.packet_size = sizeof(collision_packet);
            p.packet_type = C2H_COLLISION_PACKET;
            p.collision_type = SKILL_SKILL_COLLISION;
            p.attacker_id = m_id;
            p.victim_id = id;

            Cast<APlayerCharacter>(Owner)->do_send(&p);
            return;
        }
    }
}

void AMyWindCutter::Overlap() {
    // 나이아가라 파티클 시스템 비활성화
    if (WindCutterNiagaraComponent)
    {
        WindCutterNiagaraComponent->Deactivate();
    }

    // 히트 효과 생성
    if (WindCutterNiagaraComponent)
    {
        UNiagaraFunctionLibrary::SpawnSystemAtLocation(GetWorld(), HitEffectNiagaraSystem, GetActorLocation());
    }
    
    /*
    // 데미지 전달
    if (OtherActor->Implements<UReceiveDamageInterface>())
    {
        FSkillInfo Info;
        Info.Damage = 10.f;
        Info.Element = EClassType::CT_Wind;
        Info.StunTime = 1.5f;
        Info.KnockbackDir = (OtherActor->GetActorLocation() - GetActorLocation()).GetSafeNormal();

        // 인터페이스로 캐스팅하여 함수 호출
        IReceiveDamageInterface* DamageReceiver = Cast<IReceiveDamageInterface>(OtherActor);
        if (DamageReceiver)
        {
            DamageReceiver->ReceiveSkillHit(Info, this);
            UE_LOG(LogTemp, Warning, TEXT("Skill hit applied to: %s"), *OtherActor->GetName());
        }
    }
    */

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
