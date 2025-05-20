#include "MyStoneSkill.h"
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
        CollisionComponent->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
        CollisionComponent->SetCollisionResponseToChannel(ECC_Camera, ECR_Ignore); // 카메라 충돌 무시
        if (Owner)
        {
            CollisionComponent->IgnoreActorWhenMoving(Owner, true); // 오너와 충돌 무시
        }
    }

    FVector LaunchVelocity;
    FVector StartLocation = GetActorLocation();
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
