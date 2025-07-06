// Fill out your copyright notice in the Description page of Project Settings.

#include "MyBombActor.h"
#include "Components/SceneComponent.h"
#include "Components/BoxComponent.h"
#include "MyBoomsGimmickTrigger.h"
#include "MySkillBase.h"

// Sets default values
AMyBombActor::AMyBombActor()
{
    PrimaryActorTick.bCanEverTick = true;

    SceneComp = CreateDefaultSubobject<USceneComponent>(TEXT("SceneComp"));
    RootComponent = SceneComp;

    BoxComp = CreateDefaultSubobject<UBoxComponent>(TEXT("BoxComp"));
    BoxComp->SetupAttachment(SceneComp);
    BoxComp->SetBoxExtent(FVector(50.f));
    BoxComp->SetCollisionProfileName(TEXT("BlockAll"));
}

// Called when the game starts or when spawned
void AMyBombActor::BeginPlay()
{
	Super::BeginPlay();
	BoxComp->OnComponentBeginOverlap.AddDynamic(this, &AMyBombActor::OnBeginOverlap);
}

// Called every frame
void AMyBombActor::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}


void AMyBombActor::SetTriggerOwner(AMyBoomsGimmickTrigger* Trigger)
{
    TriggerOwner = Trigger;
}

void AMyBombActor::Destroyed()
{
    Super::Destroyed();

    if (TriggerOwner)
    {
		TriggerOwner->NotifyBombDestroyed();
    }
}

void AMyBombActor::OnBeginOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, int32 OtherBodyIndex,
	bool bFromSweep, const FHitResult& SweepResult)
{
	if (OtherActor && OtherActor->IsA(AMySkillBase::StaticClass()))
	{
		Destroy();
	}
}