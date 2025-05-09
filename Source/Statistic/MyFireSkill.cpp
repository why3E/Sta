#include "MyFireSkill.h"
#include "PlayerCharacter.h"
#include "Components/BoxComponent.h"
#include "NiagaraFunctionLibrary.h"
#include "GameFramework/Actor.h"
#include "Kismet/GameplayStatics.h"
#include "Components/PrimitiveComponent.h"
#include "NiagaraComponent.h"
#include "Enums.h"
#include "ReceiveDamageInterface.h"

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
    if (!g_is_host) { return; }
    
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

void AMyFireSkill::Overlap() {

}

void AMyFireSkill::CheckOverlappingActors()
{
    TArray<AActor*> CurrentOverlappingActors;
    CollisionComponent->GetOverlappingActors(CurrentOverlappingActors);

    for (AActor* OtherActor : CurrentOverlappingActors)
    {
        if (OtherActor)
        {
            if (OtherActor->Implements<UReceiveDamageInterface>())
            {
                FSkillInfo Info;
                Info.Damage = 10.f;
                Info.Element = EClassType::CT_Fire;
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
        }
    }
}

void AMyFireSkill::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
    Super::EndPlay(EndPlayReason);

    // 타이머 정리
    GetWorld()->GetTimerManager().ClearTimer(CheckOverlapTimerHandle);
}