#include "MyWeapon.h"
#include "Components/SphereComponent.h"
#include "Components/PoseableMeshComponent.h"
#include "GameFramework/Actor.h"
#include "GameFramework/Character.h"

// Sets default values
AMyWeapon::AMyWeapon()
{
	{
		WeaponMesh = CreateDefaultSubobject<UPoseableMeshComponent>(TEXT("WeaponMesh"));
		RootComponent = WeaponMesh;
		WeaponMesh->SetCollisionProfileName(TEXT("NoCollision"));
	}

}

// Called when the game starts or when spawned
void AMyWeapon::BeginPlay()
{
	Super::BeginPlay();
	
}

void AMyWeapon::EquipWeapon(ACharacter* Player)
{
	if (Player)
	{
		USkeletalMeshComponent* PlayerMesh = Player->GetMesh();

		WeaponMesh->AttachToComponent(PlayerMesh, FAttachmentTransformRules::KeepRelativeTransform, BaseSocketName);
	}
}

void AMyWeapon::DrawWeapon(USkeletalMeshComponent* Mesh)
{
	if (Mesh)
	{
		WeaponMesh->AttachToComponent(Mesh, FAttachmentTransformRules::KeepRelativeTransform, DrawSocketName);
	}
}

void AMyWeapon::SheatheWeapon(USkeletalMeshComponent* Mesh)
{
	if (Mesh)
	{
		WeaponMesh->AttachToComponent(Mesh, FAttachmentTransformRules::KeepRelativeTransform, BaseSocketName);
	}
}