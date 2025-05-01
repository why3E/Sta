// Fill out your copyright notice in the Description page of Project Settings.


#include "MyFireWeapon.h"
#include "MyFireBall.h"
#include "GameFramework/Character.h"
#include "Components/PoseableMeshComponent.h"
#include "Camera/CameraComponent.h"
#include "DrawDebugHelpers.h"

#include "MyPlayerVisualInterface.h"

AMyFireWeapon::AMyFireWeapon()
{
	static ConstructorHelpers::FClassFinder<AActor>FireBallRef(TEXT("/Game/Weapon/MyFireBall.MyFireBall_C"));
	if (FireBallRef.Succeeded())
	{
		FireBallClass = FireBallRef.Class;
	}

	WeaponType = EWeaponType::WT_Fire;
	BaseSocketName = TEXT("FirePosition");
	FireBallSocket = TEXT("FirePosition");
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
	TempFireBall = Cast<AMyFireBall>(GetWorld()->SpawnActor(FireBallClass));

	if (TempFireBall)
	{
		UE_LOG(LogTemp, Warning, TEXT("FireBall1 Spawned"));
		if (OwnerCharacter)
        {
            UE_LOG(LogTemp, Warning, TEXT("PlayerCharacter Spawned"));
            TempFireBall->SetOwner(OwnerCharacter);
            TempFireBall->AttachToComponent(OwnerCharacter->GetMesh(), FAttachmentTransformRules::KeepRelativeTransform, FireBallSocket);
            TempFireBall->ActivateNiagara();
        }
        else
        {
            UE_LOG(LogTemp, Error, TEXT("PlayerCharacter is null or GetOwner() is not a valid ACharacter!"));
        }

		// 발사할 위치를 선정합니다.
		SetFireLocation();
	}
	
}

void AMyFireWeapon::ShootFireBall()
{

}

void AMyFireWeapon::SetFireLocation()
{
	// 플레이어의 카메라에서 화면의 중앙으로 LineTrace를 진행합니다.
	IMyPlayerVisualInterface* PlayerCharacter = Cast<IMyPlayerVisualInterface>(GetOwner());
	if (PlayerCharacter)
	{
		// 충돌 결과 반환용
		FHitResult HitResult;
		// 시작 지점 (카메라의 위치)
		FVector Start = PlayerCharacter->GetPlayerCamera()->GetComponentLocation();
		// 종료 지점 (카메라 위치 + 카메라 전방벡터 * 20000)
		float Distance = 20000;
		FVector End = Start + (PlayerCharacter->GetPlayerCamera()->GetForwardVector() * Distance);
		// 파라미터 설정
		FCollisionQueryParams Params(SCENE_QUERY_STAT(Shoot), false, Owner);

		// 충돌 탐지
		bool bHasHit = GetWorld()->LineTraceSingleByChannel(
			HitResult,
			Start,
			End,
			ECollisionChannel::ECC_Visibility,
			Params
		);

		if (bHasHit)
		{
			FireLocation = HitResult.ImpactPoint;
		}
		else
		{
			FireLocation = End;
		}

		// 플레이어를 회전시켜줍니다.
		ACharacter* Character = Cast<ACharacter>(GetOwner());
		if (Character)
		{
			FRotator PlayerRotator = Character->GetActorRotation();
			Character->SetActorRotation(FRotator(PlayerRotator.Pitch, Character->GetControlRotation().Yaw, PlayerRotator.Roll));
		}
	}
}