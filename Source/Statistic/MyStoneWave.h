// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "MySkillBase.h"
#include "MyStoneWave.generated.h"

/**
 * 
 */
UCLASS()
class STATISTIC_API AMyStoneWave : public AMySkillBase
{
	GENERATED_BODY()
public:
	AMyStoneWave();
protected:
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
    TObjectPtr<class UBoxComponent> CollisionComponent;

    // 나이아가라 파티클 시스템
    UPROPERTY(VisibleAnywhere, Category = "Particle", meta = (AllowPrivateAccess = "true"))
    TObjectPtr<class UNiagaraComponent> StoneWaveNiagaraComponent;

    // Projectile Movement 컴포넌트
    UPROPERTY(VisibleAnywhere, Category = "Movement", meta = (AllowPrivateAccess = "true"))
    TObjectPtr<class UProjectileMovementComponent> MovementComponent;
private:
	float Speed = 2000.0f;
	uint8 bIsHit : 1;
	
	UFUNCTION()
	void OnNiagaraFinished(class UNiagaraComponent* PSystem);
public:
    virtual void Overlap(AActor* OtherActor);
    virtual void Overlap(ACharacter* OtherActor);
};
