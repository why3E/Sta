// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "MyFireBall.generated.h"

UCLASS()
class STATISTIC_API AMyFireBall : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AMyFireBall();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;
	virtual void PostInitializeComponents() override;
public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;
	void Fire(FVector TargetLocation);

	void ActivateNiagara();
protected:
    // 콜리전 처리 함수
    UFUNCTION()
    void OnBeginOverlap(class UPrimitiveComponent* OverlappedComp, class AActor* OtherActor, class UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

    // 콜리전 컴포넌트
    UPROPERTY(VisibleAnywhere, Category = "Collision", meta = (AllowPrivateAccess = "true"))
    TObjectPtr<class USphereComponent> CollisionComponent;

    // 나이아가라 파티클 시스템
    UPROPERTY(VisibleAnywhere, Category = "Particle", meta = (AllowPrivateAccess = "true"))
    TObjectPtr<class UNiagaraComponent> FireBallNiagaraComponent;

    // 히트 효과 나이아가라 파티클 시스템
    UPROPERTY(EditAnywhere, Category = "Particle", meta = (AllowPrivateAccess = "true"))
    TObjectPtr<class UNiagaraSystem> HitEffectNiagaraSystem;

    // Projectile Movement 컴포넌트
    UPROPERTY(VisibleAnywhere, Category = "Movement", meta = (AllowPrivateAccess = "true"))
    TObjectPtr<class UProjectileMovementComponent> MovementComponent;

private:
	float Speed = 3000.0f;
	uint8 bIsHit : 1;
};
