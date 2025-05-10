#include "MyWindSkill.h"
#include "PlayerCharacter.h"
#include "Components/StaticMeshComponent.h"
#include "NiagaraFunctionLibrary.h"
#include "GameFramework/Actor.h"
#include "Kismet/GameplayStatics.h"
#include "NiagaraSystem.h"
#include "MyMixWindTonado.h"
#include "ReceiveDamageInterface.h"
#include "SESSION.h"

// Sets default values
AMyWindSkill::AMyWindSkill()
{
    PrimaryActorTick.bCanEverTick = true;

    static ConstructorHelpers::FClassFinder<AMyMixWindTonado> MixBP(TEXT("/Game/Weapon/MyMixWindTonado.MyMixWindTonado_C"));
    if (MixBP.Succeeded())
    {
        MixWindTonadoClass = MixBP.Class;
    }

    // 메시 기반 콜리전 설정
    CollisionMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("CollisionMesh"));
    RootComponent = CollisionMesh;

    static ConstructorHelpers::FObjectFinder<UStaticMesh> MeshAsset(TEXT("/Game/Toon_VFX_Vol1/Meshes/SM_VFX_WindTonado.SM_VFX_WindTonado"));
    if (MeshAsset.Succeeded())
    {
        CollisionMesh->SetStaticMesh(MeshAsset.Object);
        CollisionMesh->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
        CollisionMesh->SetCollisionProfileName(TEXT("OverlapAllDynamic"));
        CollisionMesh->SetGenerateOverlapEvents(true);
        CollisionMesh->SetVisibility(false); // 보이지 않게
        CollisionMesh->SetHiddenInGame(true); // 게임 중 숨김
    }

    // 나이아가라 컴포넌트 설정
    WindTonadoNiagaraComponent = CreateDefaultSubobject<UNiagaraComponent>(TEXT("WindTonadoNiagaraComponent"));
    WindTonadoNiagaraComponent->SetupAttachment(CollisionMesh);
    WindTonadoNiagaraComponent->SetVisibility(true);
}

// Called when the game starts or when spawned
void AMyWindSkill::BeginPlay()
{
    Super::BeginPlay();

    // Tick 기반 충돌 체크 시작
    GetWorld()->GetTimerManager().SetTimer(CheckOverlapTimerHandle, this, &AMyWindSkill::CheckOverlappingActors, 1.0f, true);
}

// Called every frame
void AMyWindSkill::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
}

// 토네이도 생성 및 이펙트 실행
void AMyWindSkill::SpawnWindTonado(FVector Location)
{
    SetActorLocation(Location);

    if (WindTonadoEffect)
    {
        WindTonadoNiagaraComponent->SetAsset(WindTonadoEffect);
        WindTonadoNiagaraComponent->SetVisibility(true);
        WindTonadoNiagaraComponent->Activate();
    }

    SetLifeSpan(WindTonadoDuration);
}

void AMyWindSkill::PostInitializeComponents()
{
    Super::PostInitializeComponents();

    CollisionMesh->OnComponentBeginOverlap.AddDynamic(this, &AMyWindSkill::OnBeginOverlap);
}

void AMyWindSkill::OnBeginOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
    if (!g_is_host) return;

    if (OtherActor && OtherActor != this)
    {
        // Skill - Skill Collision
        if (OtherActor->IsA(AMySkillBase::StaticClass())) {
            if (OtherActor->IsA(AMyWindSkill::StaticClass())) {
                return;
            }
            
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

        UE_LOG(LogTemp, Warning, TEXT("Tonado hit actor: %s"), *OtherActor->GetName());
    }
}

void AMyWindSkill::Overlap()
{
    // 사용하지 않음
}

void AMyWindSkill::CheckOverlappingActors()
{
    TArray<AActor*> CurrentOverlappingActors;
    CollisionMesh->GetOverlappingActors(CurrentOverlappingActors);

    for (AActor* OtherActor : CurrentOverlappingActors)
    {
        if (OtherActor && OtherActor->Implements<UReceiveDamageInterface>())
        {
            FSkillInfo Info;
            Info.Damage = Damage;
            Info.Element = SkillElement;
            Info.StunTime = 1.5f;
            Info.KnockbackDir = (OtherActor->GetActorLocation() - GetActorLocation()).GetSafeNormal();

            IReceiveDamageInterface* DamageReceiver = Cast<IReceiveDamageInterface>(OtherActor);
            if (DamageReceiver)
            {
                DamageReceiver->ReceiveSkillHit(Info, this);
            }
        }
        if (OtherActor->Implements<UMixTonadoInterface>())
        {
            IMixTonadoInterface* MixTonado = Cast<IMixTonadoInterface>(OtherActor);
            if (MixTonado)
            {
                MixTonado->SkillMixWindTonado(SkillElement);
                Destroy();
            }
        }
    }
}

void AMyWindSkill::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
    Super::EndPlay(EndPlayReason);
    GetWorld()->GetTimerManager().ClearTimer(CheckOverlapTimerHandle);
}

void AMyWindSkill::SkillMixWindTonado(EClassType MixType)
{
    SkillElement = MixType;
    switch (MixType)
    {
    case EClassType::CT_Fire:
        if (FireEffect)
        {
            WindTonadoNiagaraComponent->SetAsset(FireEffect);
            WindTonadoNiagaraComponent->Activate(true); // 재실행
        }
        break;
    case EClassType::CT_Wind:
        SpawnMixTonado();
        Destroy();
        break;

    default:
        
        break;
    }
}

void AMyWindSkill::SpawnMixTonado()
{
    FVector SpawnLocation = GetActorLocation();

    if (MixWindTonadoClass)
    {
        AMyMixWindTonado* MixWindTonado = GetWorld()->SpawnActor<AMyMixWindTonado>(
            MixWindTonadoClass,
            SpawnLocation,
            FRotator::ZeroRotator
        );

        if (MixWindTonado)
        {
            MixWindTonado->SetActorLocation(SpawnLocation);
        }
    }
}