#include "MyFireSkill.h"
#include "PlayerCharacter.h"
#include "Components/BoxComponent.h"
#include "NiagaraFunctionLibrary.h"
#include "GameFramework/Actor.h"
#include "Kismet/GameplayStatics.h"
#include "Components/PrimitiveComponent.h"
#include "NiagaraComponent.h"

#include "SESSION.h"

// Sets default values
AMyFireSkill::AMyFireSkill()
{
    // 콜리전 컴포넌트 초기화
    CollisionComponent = CreateDefaultSubobject<UBoxComponent>(TEXT("CollisionComponent"));
    CollisionComponent->SetBoxExtent(FVector(100.0f, 100.0f, 150.0f)); // 불벽 크기 설정
    CollisionComponent->SetCollisionProfileName(TEXT("Projectile")); // 충돌 프로파일 설정
    RootComponent = CollisionComponent;

    // 나이아가라 파티클 컴포넌트 초기화
    FireWallNiagaraComponent = CreateDefaultSubobject<UNiagaraComponent>(TEXT("FireWallNiagaraComponent"));
    FireWallNiagaraComponent->SetupAttachment(CollisionComponent);
    FireWallNiagaraComponent->SetVisibility(true); 
}

// Called when the game starts or when spawned
void AMyFireSkill::BeginPlay()
{
    Super::BeginPlay();
}

// Called every frame
void AMyFireSkill::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
}

void AMyFireSkill::SpawnFireWall(FVector Location, FRotator Rotation)
{
    // Send Wind Skill Packet 
    APlayerCharacter* player = Cast<APlayerCharacter>(GetOwner());
    if (player->get_is_player()) {
        Location.Z += 75.0f;

        player_skill_packet p;
        p.packet_size = sizeof(player_skill_packet);
        p.packet_type = C2H_PLAYER_SKILL_PACKET;
        p.id = player->get_id();
        p.skill_type = SKILL_WIND_TORNADO;
        p.x = Location.X; p.y = Location.Y; p.z = Location.Z;
        player->do_send(&p);
    }
    else {
        Location = player->get_skill_location();
    }

    // 위치와 회전 설정
    SetActorLocation(Location);
    SetActorRotation(Rotation);

    // 나이아가라 파티클 활성화
    if (FireWallEffect)
    {
        FireWallNiagaraComponent->SetAsset(FireWallEffect);
        FireWallNiagaraComponent->SetVisibility(true);
        FireWallNiagaraComponent->Activate();
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
    if (OtherActor && OtherActor != this)
    {
        // 같은 클래스라면 무시
        if (OtherActor->IsA(AMyFireSkill::StaticClass()))
        {
            UE_LOG(LogTemp, Warning, TEXT("Ignored collision with another Fire Wall."));
            return;
        }

        UE_LOG(LogTemp, Warning, TEXT("Fire Wall hit actor: %s"), *OtherActor->GetName());
    }
}