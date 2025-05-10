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

void AMyWindSkill::OnBeginOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
    UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
    if (!g_is_host) return;

    if (OtherActor && OtherActor != this)
    {
        if (OtherActor->IsA(AMyWindSkill::StaticClass()))
        {
            return;
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
        UE_LOG(LogTemp, Warning, TEXT("Wind Skill"));
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

    // ✅ 미리 저장한 블루프린트 레퍼런스로 스폰
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
            UE_LOG(LogTemp, Warning, TEXT("MixWindTonado spawned at location: %s"), *SpawnLocation.ToString());
        }
        else
        {
            UE_LOG(LogTemp, Error, TEXT("Failed to spawn MixWindTonado!"));
        }
    }
    else
    {
        UE_LOG(LogTemp, Error, TEXT("MixWindTonadoClass is null! Check Blueprint reference."));
    }
}