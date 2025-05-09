#include "MyWeapon.h"
#include "Components/SphereComponent.h"
#include "Components/PoseableMeshComponent.h"
#include "GameFramework/Actor.h"
#include "GameFramework/Character.h"

// Sets default values
AMyWeapon::AMyWeapon()
{
    // WeaponCollision을 루트 컴포넌트로 설정
    WeaponCollision = CreateDefaultSubobject<USphereComponent>(TEXT("WeaponCollision"));
    RootComponent = WeaponCollision; // 루트 컴포넌트로 설정
    WeaponCollision->SetCollisionProfileName(TEXT("NoCollision"));
    WeaponCollision->bHiddenInGame = false;

    // WeaponEffect를 WeaponCollision에 부착
    WeaponEffect = CreateDefaultSubobject<UNiagaraComponent>(TEXT("WeaponEffect"));
    WeaponEffect->SetupAttachment(WeaponCollision);
}

// Called when the game starts or when spawned
void AMyWeapon::BeginPlay()
{
	Super::BeginPlay();
	WeaponCollision->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	
}


void AMyWeapon::EquipWeapon(ACharacter* Player, bool bIsLeft)
{
    if (Player)
    {
        SetOwner(Player);
        USkeletalMeshComponent* PlayerMesh = Player->GetMesh();
        OwnerCharacter = Player; // 소유자 캐릭터 설정

        // WeaponCollision을 소켓에 부착
        WeaponCollision->AttachToComponent(PlayerMesh, FAttachmentTransformRules::SnapToTargetIncludingScale, BaseLeftSocketName);

        // WeaponEffect를 WeaponCollision에 부착 (이미 설정된 경우 유지)
        WeaponEffect->AttachToComponent(WeaponCollision, FAttachmentTransformRules::KeepRelativeTransform, BaseLeftSocketName);

        // 나이아가라 효과 활성화
        WeaponEffect->Activate();
    }
}


void AMyWeapon::SheatheWeapon(USkeletalMeshComponent* Mesh)
{
    if (Mesh)
    {
        // 나이아가라 컴포넌트를 Base 소켓에 부착
        WeaponEffect->AttachToComponent(Mesh, FAttachmentTransformRules::KeepRelativeTransform, BaseLeftSocketName);
        WeaponEffect->Activate(); // 나이아가라 효과 활성화
    }
}