// Fill out your copyright notice in the Description page of Project Settings.


#include "MyFireWeapon.h"

AMyFireWeapon::AMyFireWeapon()
{
	WeaponType = EWeaponType::WT_Fire;
	BaseSocketName = TEXT("FirePosition");
}

void AMyFireWeapon::BeginPlay()
{
	Super::BeginPlay();
}

void AMyFireWeapon::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);
}

void AMyFireWeapon::SpawnFireBall()
{
	//TempEnergyBall = Cast<AFireBall>(GetWorld()->SpawnActor(EnergyBallClass));
	UE_LOG(LogTemp, Warning, TEXT("FireBall Spawned"));
	
}