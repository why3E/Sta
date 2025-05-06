// Fill out your copyright notice in the Description page of Project Settings.

#include "MyWindSkill.h"
#include "PlayerCharacter.h"
#include "Components/BoxComponent.h"
#include "NiagaraFunctionLibrary.h"
#include "GameFramework/Actor.h"
#include "Kismet/GameplayStatics.h"
#include "Components/PrimitiveComponent.h"
#include "NiagaraSystem.h"

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
}

// Called every frame
void AMyWindSkill::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

void AMyWindSkill::SpawnWindTonado(FVector Location)
{
    // Send Wind Skill Packet 
    APlayerCharacter* player = Cast<APlayerCharacter>(GetOwner());
    if (player->get_is_player()) {
        Location.Z += 375.0f;
        
        player_skill_packet p;
        p.packet_size = sizeof(player_skill_packet);
        p.packet_type = C2H_PLAYER_SKILL_PACKET;
        p.id = player->get_id();
        p.skill_type = SKILL_WIND_TORNADO;
        p.x = Location.X; p.y = Location.Y; p.z = Location.Z;
        player->do_send(&p);
    }
    else {
        Location = player->get_skill_location();
    }

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
            UE_LOG(LogTemp, Warning, TEXT("Ignored collision with another Fire Wall."));
            return;
        }

        UE_LOG(LogTemp, Warning, TEXT("Fire Wall hit actor: %s"), *OtherActor->GetName());
    }
}