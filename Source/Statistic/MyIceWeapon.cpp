// Fill out your copyright notice in the Description page of Project Settings.


#include "MyIceWeapon.h"

AMyIceWeapon::AMyIceWeapon()
{
    // WeaponMesh 생성 및 WeaponCollision(부모 루트)에 부착
    WeaponMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("WeaponMesh"));
    WeaponMesh->SetupAttachment(RootComponent); // WeaponCollision에 부착

    // 필요하다면 메시의 충돌 설정 등 추가
    WeaponMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
    WeaponMesh->SetVisibility(true);

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