#include "MyBombAttack.h"
#include "ReceiveDamageInterface.h"
#include "SESSION.h"
#include "Components/StaticMeshComponent.h"
#include "NiagaraComponent.h"
#include "NiagaraSystem.h"
#include "Kismet/GameplayStatics.h"

// Sets default values
// Sets default values
// Sets default values
AMyBombAttack::AMyBombAttack()
{
    PrimaryActorTick.bCanEverTick = true;

    // ✔️ Root를 따로 만들기
    USceneComponent* Root = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));
    RootComponent = Root;

    // ✔️ 콜리전 메시
    CollisionMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("CollisionMesh"));
    CollisionMesh->SetupAttachment(Root);

    static ConstructorHelpers::FObjectFinder<UStaticMesh> MeshAsset(TEXT("/Game/Toon_VFX_Vol1/Meshes/SM_VFX_Boom.SM_VFX_Boom"));
    if (MeshAsset.Succeeded())
    {
        CollisionMesh->SetStaticMesh(MeshAsset.Object);
    }

    // 👉 CollisionMesh만 스케일 2배
    CollisionMesh->SetWorldScale3D(FVector(2.0f, 2.0f, 2.0f));

    CollisionMesh->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
    CollisionMesh->SetCollisionProfileName(TEXT("OverlapAllDynamic"));
    CollisionMesh->SetGenerateOverlapEvents(true);

    CollisionMesh->SetVisibility(false);
    CollisionMesh->SetHiddenInGame(true);

    // ✔️ Niagara는 Root에 바로 붙인다 (CollisionMesh 영향 X)
    MixBombAttackNiagaraComponent = CreateDefaultSubobject<UNiagaraComponent>(TEXT("MixBombAttackNiagaraComponent"));
    MixBombAttackNiagaraComponent->SetupAttachment(Root);
    MixBombAttackNiagaraComponent->SetVisibility(true);
}



// Called when the game starts or when spawned
void AMyBombAttack::BeginPlay()
{
    Super::BeginPlay();

    // Tick 기반 충돌 체크 시작
    GetWorld()->GetTimerManager().SetTimerForNextTick([this]()
    {
        GetWorld()->GetTimerManager().SetTimer(
            CheckOverlapTimerHandle, this,
            &AMyBombAttack::CheckOverlappingActors,
            1.0f, true
        );
    });
}

// Called every frame
void AMyBombAttack::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
}

// 폭탄 스킬 생성 함수
void AMyBombAttack::SpawnBombAttack(FVector ImpactPoint, EClassType MixType)
{
    SetActorLocation(ImpactPoint);
    SkillElement = MixType;

    if (MixType == EClassType::CT_Fire && FireEffect)
    {
        MixBombAttackNiagaraComponent->SetAsset(FireEffect);
    }
    else if (MixBombAttackEffect)
    {
        MixBombAttackNiagaraComponent->SetAsset(MixBombAttackEffect);
    }

    MixBombAttackNiagaraComponent->Activate(true);
    SetLifeSpan(3.75f);    // 3.75초 후 파괴
}

void AMyBombAttack::PostInitializeComponents()
{
    Super::PostInitializeComponents();

    CollisionMesh->OnComponentBeginOverlap.AddDynamic(this, &AMyBombAttack::OnBeginOverlap);
}

void AMyBombAttack::OnBeginOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult) {
    if (!g_is_host) return;

    if (OtherActor && OtherActor != this) {
        UE_LOG(LogTemp, Warning, TEXT("Bomb hit actor: %s"), *OtherActor->GetName());
    }
}

void AMyBombAttack::Overlap(AActor* OtherActor) {
    if (OtherActor && OtherActor->Implements<UReceiveDamageInterface>()) {
        FSkillInfo Info;
        Info.Damage = SkillDamage;
        Info.Element = SkillElement;
        Info.StunTime = 1.5f;
        Info.KnockbackDir = (OtherActor->GetActorLocation() - GetActorLocation()).GetSafeNormal();

        IReceiveDamageInterface* DamageReceiver = Cast<IReceiveDamageInterface>(OtherActor);
        if (DamageReceiver) {
            DamageReceiver->ReceiveSkillHit(Info, this);
        }
    }
}

void AMyBombAttack::CheckOverlappingActors() {

}

void AMyBombAttack::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
    Super::EndPlay(EndPlayReason);
    GetWorld()->GetTimerManager().ClearTimer(CheckOverlapTimerHandle);
}
