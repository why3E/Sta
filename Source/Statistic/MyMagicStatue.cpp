// Fill out your copyright notice in the Description page of Project Settings.


#include "MyMagicStatue.h"
#include "Components/BoxComponent.h"
#include "PlayerCharacter.h"
#include "EngineUtils.h" 


// Sets default values
AMyMagicStatue::AMyMagicStatue()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	sceneComp = CreateDefaultSubobject<USceneComponent>(TEXT("sceneComp"));
	RootComponent = sceneComp;

	boxCollision = CreateDefaultSubobject<UBoxComponent>(TEXT("BoxCollision"));
	boxCollision->SetupAttachment(RootComponent);

}

// Called when the game starts or when spawned
void AMyMagicStatue::BeginPlay()
{
	Super::BeginPlay();
	boxCollision->OnComponentBeginOverlap.AddDynamic(this, &AMyMagicStatue::OnBeginOverlapCollision);
}

// Called every frame
void AMyMagicStatue::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

void AMyMagicStatue::OnBeginOverlapCollision(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
    APlayerCharacter* Player = Cast<APlayerCharacter>(OtherActor);
    if (Player && Player->GetController())
    {
        // 중복 텔레포트 방지
        if (Player->bRecentlyTeleported) return;

        UWorld* World = GetWorld();
        if (!World) return;

        int32 NextStatueNumber = StatueNumber + 1;
        AMyMagicStatue* NextStatue = nullptr;
        AMyMagicStatue* FirstStatue = nullptr;
        int32 MinStatueNumber = INT32_MAX;

        for (TActorIterator<AMyMagicStatue> It(World); It; ++It)
        {
            AMyMagicStatue* Statue = *It;
            if (!Statue) continue;
            if (Statue->StatueNumber < MinStatueNumber)
            {
                MinStatueNumber = Statue->StatueNumber;
                FirstStatue = Statue;
            }
            if (Statue != this && Statue->StatueNumber == NextStatueNumber)
            {
                NextStatue = Statue;
            }
        }

        FVector TargetLocation;
        if (NextStatue)
        {
            FVector Forward = NextStatue->GetActorForwardVector();
            TargetLocation = NextStatue->GetActorLocation() + Forward * 500.0f;
        }
        else if (FirstStatue)
        {
            FVector Forward = FirstStatue->GetActorForwardVector();
            TargetLocation = FirstStatue->GetActorLocation() + Forward * 500.0f;
        }
        else
        {
            return;
        }

        // 라인트레이스로 지형 Z값 확인
        FVector TraceStart = TargetLocation + FVector(0, 0, 500.0f);
        FVector TraceEnd = TargetLocation - FVector(0, 0, 500.0f);
        FHitResult HitResult;
        FCollisionQueryParams Params;
        Params.AddIgnoredActor(Player);

        float FinalZ = TargetLocation.Z;
        if (World->LineTraceSingleByChannel(HitResult, TraceStart, TraceEnd, ECC_Visibility, Params))
        {
            FinalZ = HitResult.ImpactPoint.Z;
        }
        // 캐릭터가 끼지 않게 Z값에 100 추가
        TargetLocation.Z = FinalZ + 100.0f;

        Player->SetActorLocation(TargetLocation);

        // 최근 텔레포트한 플레이어 저장 및 쿨타임 후 초기화
        Player->bRecentlyTeleported = true;
        World->GetTimerManager().SetTimer(
            TeleportCooldownHandle,
            [Player]()
            {
                if (Player) Player->bRecentlyTeleported = false;
            },
            0.5f, // 0.5초 후 해제
            false
        );
    }
}