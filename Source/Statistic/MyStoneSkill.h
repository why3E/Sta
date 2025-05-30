// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "MySkillBase.h"
#include "MyStoneSkill.generated.h"

/**
 * 
 */
UCLASS()
class STATISTIC_API AMyStoneSkill : public AMySkillBase
{
	GENERATED_BODY()
	
public:	
	AMyStoneSkill();

protected:
	virtual void BeginPlay() override;
	virtual void PostInitializeComponents() override;

public:
	virtual void Tick(float DeltaTime) override;

    // 돌을 던지는 함수
    void Fire(FVector FireLocation);

    virtual void Overlap(char skill_type) override;
    virtual void Overlap(unsigned short object_id, bool collision_start) override;

protected:
    // 콜리전 처리 함수
    UFUNCTION()
    void OnBeginOverlap(class UPrimitiveComponent* OverlappedComp, class AActor* OtherActor, class UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

protected:
    // 돌 메시
    UPROPERTY(VisibleAnywhere, Category = "Stone")
    UStaticMeshComponent* StoneMesh;

    // 콜리전
    UPROPERTY(VisibleAnywhere, Category = "Collision")
    class USphereComponent* CollisionComponent;

    // 무브먼트(던지기)
    UPROPERTY(VisibleAnywhere, Category = "Movement")
    class UProjectileMovementComponent* ProjectileMovement;

    // 날아갈 때 붙일 나이아가라 이펙트
    UPROPERTY(VisibleAnywhere, Category = "Effect")
    class UNiagaraComponent* TrailNiagaraComponent;

private:
	float Speed = 3000.0f;
	uint8 bIsHit : 1;

    
};
