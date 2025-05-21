// Fill out your copyright notice in the Description page of Project Settings.


#include "MyIceWeapon.h"
#include "MyIceArrow.h"
#include "PlayerCharacter.h"
#include "GameFramework/Character.h"

AMyIceWeapon::AMyIceWeapon()
{
    // WeaponMesh 생성 및 WeaponCollision(부모 루트)에 부착
    WeaponMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("WeaponMesh"));
    WeaponMesh->SetupAttachment(RootComponent); // WeaponCollision에 부착

    // 필요하다면 메시의 충돌 설정 등 추가
    WeaponMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
    WeaponMesh->SetVisibility(false);

    static ConstructorHelpers::FClassFinder<AActor> IceArrowRef(TEXT("/Game/Weapon/MyIceArrow.MyIceArrow_C"));
    if (IceArrowRef.Succeeded())
    {
        IceArrowClass =  IceArrowRef.Class;
    }

    WeaponType = EWeaponType::WT_Ice;

    IceSocket = TEXT("IcePosition");
}

void AMyIceWeapon::BeginPlay()
{
    Super::BeginPlay();

}
void AMyIceWeapon::Tick(float DeltaSeconds)
{
    Super::Tick(DeltaSeconds);

}

void AMyIceWeapon::SetAiming()
{
    if (bIsAiming)
        return;
    bIsAiming = true;
    WeaponMesh->SetVisibility(true);
   TempIceArrow = Cast<AMyIceArrow>(GetWorld()->SpawnActor(IceArrowClass));
    UE_LOG(LogTemp, Error, TEXT("FireBall %d Spawning"), Cast<APlayerCharacter>(OwnerCharacter)->get_skill_id());

    if (TempIceArrow)
	{
		if (OwnerCharacter)
        {
            UE_LOG(LogTemp, Warning, TEXT("Ice Arrow Spawned"));
            // 소유자 설정
            TempIceArrow->SetOwner(OwnerCharacter);

            // 소켓에 부착
            TempIceArrow->AttachToComponent(OwnerCharacter->GetMesh(), FAttachmentTransformRules::KeepRelativeTransform, IceSocket);

            // 파티클 등 활성화
            TempIceArrow->ActivateNiagara();
        }
    }
}


void AMyIceWeapon::ShootIceArrow(FVector FirePoint)
{
    if (TempIceArrow)
	{
		// 부모 액터로부터 부착 해제
		TempIceArrow->DetachFromActor(FDetachmentTransformRules::KeepWorldTransform);
		TempIceArrow->Fire(FirePoint);
		TempIceArrow = nullptr;
	}
    bIsAiming = false;
    WeaponMesh->SetVisibility(false);
}

