// Fill out your copyright notice in the Description page of Project Settings.
#include "MyWindWeapon.h"
#include "MyWindCutter.h"
#include "GameFramework/Character.h"
#include "Components/PoseableMeshComponent.h"
#include "Camera/CameraComponent.h"
#include "DrawDebugHelpers.h"
#include "MyPlayerVisualInterface.h"

AMyWindWeapon::AMyWindWeapon()
{
    static ConstructorHelpers::FClassFinder<AActor>WindCutterRef(TEXT("/Game/Weapon/MyWindCutter.MyWindCutter_C"));
    if (WindCutterRef.Succeeded())
    {
        WindCutterClass = WindCutterRef.Class;
        
        UE_LOG(LogTemp, Error, TEXT("find WindCutter class!"));
    }
    else
    {
        UE_LOG(LogTemp, Error, TEXT("Failed to find WindCutter class!"));
    }
    WeaponType = EWeaponType::WT_Wind;
    BaseSocketName = TEXT("WindPosition");
    WindCutterSocket = TEXT("WindPosition");
}


void AMyWindWeapon::BeginPlay()
{
	Super::BeginPlay();
}

void AMyWindWeapon::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);
}

void AMyWindWeapon::SpawnWindCutter()
{
	TempWindCutter = Cast<AMyWindCutter>(GetWorld()->SpawnActor(WindCutterClass));
    
	if (TempWindCutter)
	{
		UE_LOG(LogTemp, Warning, TEXT("WindCutter Spawned"));
		if (OwnerCharacter)
        {
            UE_LOG(LogTemp, Warning, TEXT("PlayerCharacter Spawned"));
            TempWindCutter->SetOwner(OwnerCharacter);
            TempWindCutter->AttachToComponent(OwnerCharacter->GetMesh(), FAttachmentTransformRules::KeepRelativeTransform, WindCutterSocket);
            TempWindCutter->ActivateNiagara();
        }
        else
        {
            UE_LOG(LogTemp, Error, TEXT("PlayerCharacter is null or GetOwner() is not a valid ACharacter!"));
        }

		// 발사할 위치를 선정합니다.
		SetFireLocation();
	}
	
}

void AMyWindWeapon::ShootWindCutter()
{
	if (TempWindCutter)
	{
		// 부모 액터로부터 부착 해제
		TempWindCutter->DetachFromActor(FDetachmentTransformRules::KeepWorldTransform);

		TempWindCutter->Fire(FireLocation);
		TempWindCutter = nullptr;
	}
}

void AMyWindWeapon::SetFireLocation()
{
	// 플레이어의 카메라에서 화면의 중앙으로 LineTrace를 진행합니다.
	IMyPlayerVisualInterface* PlayerCharacter = Cast<IMyPlayerVisualInterface>(GetOwner());

	UE_LOG(LogTemp, Warning, TEXT("PlayerCharacter: %s"), PlayerCharacter ? TEXT("Valid") : TEXT("Invalid"));
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
		UE_LOG(LogTemp, Warning, TEXT("FireLocation: %s"), *FireLocation.ToString());
\
		ACharacter* Character = Cast<ACharacter>(GetOwner());
		if (Character)
		{
			FRotator PlayerRotator = Character->GetActorRotation();
			Character->SetActorRotation(FRotator(PlayerRotator.Pitch, Character->GetControlRotation().Yaw, PlayerRotator.Roll));
		}
	}
}