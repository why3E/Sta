#include "MyFireSkill.h"
#include "MyWindSkill.h"
#include "EnemyCharacter.h"
#include "PlayerCharacter.h"
#include "Components/BoxComponent.h"
#include "NiagaraFunctionLibrary.h"
#include "GameFramework/Actor.h"
#include "Kismet/GameplayStatics.h"
#include "Components/PrimitiveComponent.h"
#include "NiagaraComponent.h"
#include "Enums.h"
#include "ReceiveDamageInterface.h"
#include "MixTonadoInterface.h"

#include "SESSION.h"

AMyFireSkill::AMyFireSkill()
{
    SetElement(EClassType::CT_Fire);

    // 콜리전 컴포넌트 초기화
    CollisionComponent = CreateDefaultSubobject<UBoxComponent>(TEXT("CollisionComponent"));
    CollisionComponent->SetBoxExtent(FVector(100.0f, 100.0f, 150.0f)); // 불벽 크기 설정
    CollisionComponent->SetCollisionProfileName(TEXT("Projectile")); // 충돌 프로파일 설정
    RootComponent = CollisionComponent;

    // 기존 불벽 나이아가라 컴포넌트 초기화
    FireWallNiagaraComponent = CreateDefaultSubobject<UNiagaraComponent>(TEXT("FireWallNiagaraComponent"));
    FireWallNiagaraComponent->SetupAttachment(CollisionComponent);
    FireWallNiagaraComponent->SetVisibility(true);

    // 추가적인 시각적 효과용 나이아가라 컴포넌트 초기화
    FireWallVisualEffectComponent = CreateDefaultSubobject<UNiagaraComponent>(TEXT("FireWallVisualEffectComponent"));
    FireWallVisualEffectComponent->SetupAttachment(FireWallNiagaraComponent); // 기존 컴포넌트에 부착
    FireWallVisualEffectComponent->SetVisibility(true);
}

// Called when the game starts or when spawned
void AMyFireSkill::BeginPlay()
{
    Super::BeginPlay();

    

    GetWorld()->GetTimerManager().SetTimer(CheckOverlapTimerHandle, this, &AMyFireSkill::CheckOverlappingActors, 1.0f, true);
}

// Called every frame
void AMyFireSkill::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
}

void AMyFireSkill::SpawnFireWall(FVector Location, FRotator Rotation)
{
    // 위치와 회전 설정
    SetActorLocation(Location + FVector(0.0f, 0.0f, 75.0f)); // 불벽이 땅 위에 위치하도록 조정
    SetActorRotation(Rotation);

    // 기존 불벽 나이아가라 파티클 활성화
    if (FireWallEffect && FireWallNiagaraComponent)
    {
        FireWallNiagaraComponent->SetAsset(FireWallEffect);
        FireWallNiagaraComponent->SetVisibility(true);
        FireWallNiagaraComponent->Activate();
    }

    // 추가적인 시각적 효과용 나이아가라 파티클 활성화
    if (FireWallVisualEffectSystem && FireWallVisualEffectComponent)
    {
        FireWallVisualEffectComponent->SetAsset(FireWallVisualEffectSystem);
        FireWallVisualEffectComponent->SetVisibility(true);
        FireWallVisualEffectComponent->Activate();
    }

    // 불벽의 지속 시간 설정
    SetLifeSpan(FireWallDuration);

    UE_LOG(LogTemp, Warning, TEXT("Fire Wall spawned at location: %s"), *Location.ToString());
}

void AMyFireSkill::PostInitializeComponents()
{
    Super::PostInitializeComponents();

    // Event Mapping
    CollisionComponent->OnComponentBeginOverlap.AddDynamic(this, &AMyFireSkill::OnBeginOverlap);
}

void AMyFireSkill::OnBeginOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
    if (!g_is_host) { return; }
    
    if (OtherActor && OtherActor != this)
    {
        if (OtherActor->IsA(AMySkillBase::StaticClass())) {
            if (OtherActor->IsA(AMyFireSkill::StaticClass())) { 
                return; 
            }

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
                    UE_LOG(LogTemp, Warning, TEXT("Wall ID : %d"), m_id);
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
        }

        UE_LOG(LogTemp, Warning, TEXT("Fire Wall hit actor: %s"), *OtherActor->GetName());
    }
}

void AMyFireSkill::Overlap(AActor* OtherActor) {
    if (OtherActor && OtherActor->IsA(AMyWindSkill::StaticClass())) {
        Destroy();
    }
}

void AMyFireSkill::Overlap(ACharacter* OtherActor) {

}

void AMyFireSkill::CheckOverlappingActors() {

}

void AMyFireSkill::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
    Super::EndPlay(EndPlayReason);

    // 타이머 정리
    GetWorld()->GetTimerManager().ClearTimer(CheckOverlapTimerHandle);
}