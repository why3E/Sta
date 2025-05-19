// Fill out your copyright notice in the Description page of Project Settings.


#include "MyStoneWeapon.h"


AMyStoneWeapon::AMyStoneWeapon()
{
    // 필요하다면 초기화 코드 작성
    static ConstructorHelpers::FClassFinder<AActor>StoneWaveRef(TEXT("/Game/Weapon/MyStoneWave.MyStoneWave_C"));
    if (StoneWaveRef.Succeeded())
    {
        StoneWaveClass = StoneWaveRef.Class;
    }
}

void AMyStoneWeapon::BeginPlay()
{
	Super::BeginPlay();
}

void AMyStoneWeapon::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);
}

void AMyStoneWeapon::SpawnStoneWave(FVector FireLocation)
{
    if (StoneWaveClass)
    {   
        FVector SpawnLocation = GetOwner() ? GetOwner()->GetActorLocation() : FVector::ZeroVector;
        FActorSpawnParameters SpawnParams;
        SpawnParams.Owner = this;
        SpawnParams.Instigator = GetInstigator();

        // 소환 위치는 Owner의 위치, Fire 함수에는 목표 위치(FireLocation) 전달
        AMyStoneWave* StoneWave = GetWorld()->SpawnActor<AMyStoneWave>(StoneWaveClass, SpawnLocation, FRotator::ZeroRotator, SpawnParams);
        if (StoneWave)
        {
            StoneWave->Fire(FireLocation);
        }
    }
}

void AMyStoneWeapon::SpawnStoneSkill(FVector TargetLocation)
{
    
}