// Fill out your copyright notice in the Description page of Project Settings.

#include "MyWindSkill.h"
#include "PlayerCharacter.h"
#include "Components/BoxComponent.h"
#include "NiagaraFunctionLibrary.h"
#include "GameFramework/Actor.h"
#include "Kismet/GameplayStatics.h"
#include "Components/PrimitiveComponent.h"
#include "NiagaraSystem.h"
#include "ReceiveDamageInterface.h"

#include "SESSION.h"

// Sets default values
AMyWindSkill::AMyWindSkill()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	 CollisionComponent = CreateDefaultSubobject<UBoxComponent>(TEXT("CollisionComponent"));
	 CollisionComponent->SetBoxExtent(FVector(300.0f, 300.0f, 375.0f)); 
	 CollisionComponent->SetCollisionProfileName(TEXT("CustomCollision"));
	 RootComponent = CollisionComponent;
 
	 // 나이아가라 파티클 컴포넌트 초기화
	 WindTonadoNiagaraComponent = CreateDefaultSubobject<UNiagaraComponent>(TEXT("WindTonadoNiagaraComponent"));
	 WindTonadoNiagaraComponent->SetupAttachment(CollisionComponent);
	 WindTonadoNiagaraComponent->SetVisibility(true); 

}

// Called when the game starts or when spawned
void AMyWindSkill::BeginPlay()
{
	Super::BeginPlay();
  GetWorld()->GetTimerManager().SetTimer(CheckOverlapTimerHandle, this, &AMyWindSkill::CheckOverlappingActors, 1.0f, true);

}

// Called every frame
void AMyWindSkill::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

void AMyWindSkill::SpawnWindTonado(FVector Location)
{
    SetActorLocation(Location);

    // 나이아가라 파티클 활성화
    if (WindTonadoEffect)
    {
        WindTonadoNiagaraComponent->SetAsset(WindTonadoEffect);
        WindTonadoNiagaraComponent->SetVisibility(true);
        WindTonadoNiagaraComponent->Activate();
    }

    // 불벽의 지속 시간 설정
    SetLifeSpan(WindTonadoDuration);
}

void AMyWindSkill::PostInitializeComponents()
{
    Super::PostInitializeComponents();

    // Event Mapping
    CollisionComponent->OnComponentBeginOverlap.AddDynamic(this, &AMyWindSkill::OnBeginOverlap);
}

void AMyWindSkill::OnBeginOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
    if (OtherActor && OtherActor != this)
    {
        // 같은 클래스라면 무시
        if (OtherActor->IsA(AMyWindSkill::StaticClass()))
        {
            return;
        }

        UE_LOG(LogTemp, Warning, TEXT("Tonado hit actor: %s"), *OtherActor->GetName());
    }
}


void AMyWindSkill::CheckOverlappingActors()
{
    TArray<AActor*> CurrentOverlappingActors;
    CollisionComponent->GetOverlappingActors(CurrentOverlappingActors);

    for (AActor* OtherActor : CurrentOverlappingActors)
    {
        if (OtherActor)
        {
            if (OtherActor->Implements<UReceiveDamageInterface>())
            {
                FSkillInfo Info;
                Info.Damage = 10.f;
                Info.Element = EClassType::CT_Wind;
                Info.StunTime = 1.5f;
                Info.KnockbackDir = (OtherActor->GetActorLocation() - GetActorLocation()).GetSafeNormal();

                // 인터페이스로 캐스팅하여 함수 호출
                IReceiveDamageInterface* DamageReceiver = Cast<IReceiveDamageInterface>(OtherActor);
                if (DamageReceiver)
                {
                    DamageReceiver->ReceiveSkillHit(Info, this);
                    UE_LOG(LogTemp, Warning, TEXT("Skill hit applied to: %s"), *OtherActor->GetName());
                }
            }
        }
    }
}

void AMyWindSkill::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
    Super::EndPlay(EndPlayReason);

    // 타이머 정리
    GetWorld()->GetTimerManager().ClearTimer(CheckOverlapTimerHandle);
}