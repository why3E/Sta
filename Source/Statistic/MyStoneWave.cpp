#include "MyStoneWave.h"
#include "Components/BoxComponent.h"
#include "NiagaraComponent.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "Kismet/GameplayStatics.h"

AMyStoneWave::AMyStoneWave()
{
    PrimaryActorTick.bCanEverTick = true;

    // 콜리전 컴포넌트 생성 및 루트로 설정
    CollisionComponent = CreateDefaultSubobject<UBoxComponent>(TEXT("CollisionComponent"));
    RootComponent = CollisionComponent;
    CollisionComponent->SetCollisionProfileName(TEXT("OverlapAllDynamic"));
    CollisionComponent->SetGenerateOverlapEvents(true);

    // 나이아가라 컴포넌트 생성
    StoneWaveNiagaraComponent = CreateDefaultSubobject<UNiagaraComponent>(TEXT("StoneWaveNiagaraComponent"));
    StoneWaveNiagaraComponent->SetupAttachment(RootComponent);

    // 무브먼트 컴포넌트 생성
    MovementComponent = CreateDefaultSubobject<UProjectileMovementComponent>(TEXT("MovementComponent"));
    MovementComponent->InitialSpeed = Speed;
    MovementComponent->MaxSpeed = Speed;
    MovementComponent->bRotationFollowsVelocity = true;
    MovementComponent->ProjectileGravityScale = 0.f;
}

void AMyStoneWave::PostInitializeComponents()
{
    Super::PostInitializeComponents();
    if (CollisionComponent)
    {
        CollisionComponent->OnComponentBeginOverlap.AddDynamic(this, &AMyStoneWave::OnBeginOverlap);
    }
}

void AMyStoneWave::BeginPlay()
{
    Super::BeginPlay();
    ActivateNiagara();

    if (StoneWaveNiagaraComponent)
    {
        StoneWaveNiagaraComponent->OnSystemFinished.AddDynamic(this, &AMyStoneWave::OnNiagaraFinished);
    }
}

void AMyStoneWave::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    // 현재 위치보다 위에서 시작 → 바닥까지 검사
    FVector Start = GetActorLocation() + FVector(0, 0, 200.f);
    FVector End = Start - FVector(0, 0, 5000.0f);

    FHitResult HitResult;
    FCollisionQueryParams Params;
    Params.AddIgnoredActor(this);

    bool bHit = GetWorld()->LineTraceSingleByChannel(
        HitResult,
        Start,
        End,
        ECC_Visibility,
        Params
    );

    if (bHit)
    {
        // 무조건 바닥 위치 기준으로 +35 위로 올림
        FVector Location = GetActorLocation();
        Location.Z = HitResult.ImpactPoint.Z + 35.f;
        SetActorLocation(Location);

        // Niagara도 같이 올리려면 이 부분 추가
        if (StoneWaveNiagaraComponent)
        {
            FVector NiagaraLocation = StoneWaveNiagaraComponent->GetComponentLocation();
            NiagaraLocation.Z = HitResult.ImpactPoint.Z + 35.f;
            StoneWaveNiagaraComponent->SetWorldLocation(NiagaraLocation);
        }
    }

    // 감속
    if (MovementComponent)
    {
        float Damping = 0.98f;
        MovementComponent->Velocity *= Damping;
    }
}


void AMyStoneWave::Fire(FVector TargetLocation)
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

}

void AMyStoneWave::ActivateNiagara()
{
    if (StoneWaveNiagaraComponent)
    {
        StoneWaveNiagaraComponent->Activate(true);
    }
}
void AMyStoneWave::OnNiagaraFinished(UNiagaraComponent* PSystem)
{
    Destroy();
}
void AMyStoneWave::OnBeginOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{

}

void AMyStoneWave::Overlap(AActor* OtherActor)
{
}

void AMyStoneWave::Overlap(ACharacter* OtherActor)
{
}