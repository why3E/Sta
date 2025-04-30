#include "MyWeapon.h"
#include "Components/SphereComponent.h"
#include "Components/PoseableMeshComponent.h"
#include "GameFramework/Actor.h"
#include "GameFramework/Character.h"

// Sets default values
AMyWeapon::AMyWeapon()
{
	
		{
			WeaponEffect = CreateDefaultSubobject<UNiagaraComponent>(TEXT("WeaponEffect"));
			WeaponEffect->SetupAttachment(RootComponent);
	
			WeaponCollision = CreateDefaultSubobject<USphereComponent>(TEXT("WeaponCollision"));
			WeaponCollision->SetupAttachment(RootComponent);
			WeaponCollision->SetCollisionProfileName(TEXT("NoCollision"));
			WeaponCollision->bHiddenInGame = false;
		}
	

}

// Called when the game starts or when spawned
void AMyWeapon::BeginPlay()
{
	Super::BeginPlay();
	WeaponCollision->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	
}


void AMyWeapon::EquipWeapon(ACharacter* Player)
{
    if (Player)
    {
        USkeletalMeshComponent* PlayerMesh = Player->GetMesh();

        WeaponEffect->AttachToComponent(PlayerMesh, FAttachmentTransformRules::SnapToTargetIncludingScale, BaseSocketName);
        WeaponEffect->Activate(); // 나이아가라 효과 활성화
    }
}


void AMyWeapon::SheatheWeapon(USkeletalMeshComponent* Mesh)
{
    if (Mesh)
    {
        // 나이아가라 컴포넌트를 Base 소켓에 부착
        WeaponEffect->AttachToComponent(Mesh, FAttachmentTransformRules::KeepRelativeTransform, BaseSocketName);
        WeaponEffect->Activate(); // 나이아가라 효과 활성화
    }
}