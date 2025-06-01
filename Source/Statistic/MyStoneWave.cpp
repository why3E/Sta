#include "MyStoneWave.h"
#include "MyEnemyBase.h"
#include "PlayerCharacter.h"
#include "Components/BoxComponent.h"
#include "NiagaraComponent.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "Kismet/GameplayStatics.h"
#include "DrawDebugHelpers.h"

AMyStoneWave::AMyStoneWave()
{
	SetElement(EClassType::CT_Stone);
	SetType(SKILL_STONE_WAVE);

	PrimaryActorTick.bCanEverTick = true;

	CollisionComponent = CreateDefaultSubobject<UBoxComponent>(TEXT("CollisionComponent"));
	RootComponent = CollisionComponent;
	CollisionComponent->SetCollisionProfileName(TEXT("OverlapAllDynamic"));
	CollisionComponent->SetGenerateOverlapEvents(true);

	StoneWaveNiagaraComponent = CreateDefaultSubobject<UNiagaraComponent>(TEXT("StoneWaveNiagaraComponent"));
	StoneWaveNiagaraComponent->SetupAttachment(RootComponent);

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

	StartDestroyTimer();
}

void AMyStoneWave::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	FVector Start = GetActorLocation() + FVector(0, 0, 200.f);
	FVector End = Start - FVector(0, 0, 5000.f);

	FHitResult HitResult;
	FCollisionQueryParams Params;
	Params.AddIgnoredActor(this);

	if (GetWorld()->LineTraceSingleByChannel(HitResult, Start, End, ECC_Visibility, Params))
{
	FVector NewLocation = GetActorLocation();
	NewLocation.Z = HitResult.ImpactPoint.Z + 2.0f;
	SetActorLocation(NewLocation);

	if (StoneWaveNiagaraComponent && !MovementComponent->Velocity.IsNearlyZero())
	{
		FVector NiagaraLoc = NewLocation;
		FVector MoveDir = MovementComponent->Velocity.GetSafeNormal();
		FRotator NiagaraRotation = MoveDir.Rotation();
		NiagaraRotation.Pitch = 0.f;  // 바닥 기준으로만 회전
		NiagaraRotation.Roll = 0.f;

		StoneWaveNiagaraComponent->SetWorldLocation(NiagaraLoc);
		StoneWaveNiagaraComponent->SetWorldRotation(NiagaraRotation);
	}
}

	if (MovementComponent)
	{
		MovementComponent->Velocity *= 0.98f;
	}
}

void AMyStoneWave::Fire(FVector TargetLocation)
{
	if (!Owner) return;

	FVector LaunchDirection = (TargetLocation - GetActorLocation()).GetSafeNormal();
	LaunchDirection.Z = 0.0f;

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

void AMyStoneWave::StartDestroyTimer()
{
	GetWorld()->GetTimerManager().SetTimer(DestroyTimerHandle, this, &AMyStoneWave::DestroySelf, 1.5f, false);
}

void AMyStoneWave::DestroySelf()
{
	Destroy();
}

void AMyStoneWave::OnNiagaraFinished(UNiagaraComponent* PSystem)
{
	// Destroy(); // Timer가 따로 관리하므로 이건 생략 가능
}

void AMyStoneWave::OnBeginOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult) {
	if (!g_is_host || !OtherActor || OtherActor == this) return;

	if (OtherActor->IsA(AMySkillBase::StaticClass())) {
		AMySkillBase* ptr = Cast<AMySkillBase>(OtherActor);

		if (g_c_skills.count(ptr->m_id)) {
			if (m_id < ptr->m_id) {
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
		AMyEnemyBase* ptr = Cast<AMyEnemyBase>(OtherActor);

		if (g_c_monsters.count(ptr->get_id())) {
			if (ptr->GetHP() > 0.0f) {
				{
					CollisionEvent collision_event = MonsterSkillEvent(ptr->get_id(), GetType(), GetActorLocation());
					std::lock_guard<std::mutex> lock(g_s_collision_events_l);
					g_s_collision_events.push(collision_event);
				}
			}
		}
	}
}

void AMyStoneWave::Overlap(char skill_type) {

}

void AMyStoneWave::Overlap(unsigned short object_id, bool collision_start) {

}
