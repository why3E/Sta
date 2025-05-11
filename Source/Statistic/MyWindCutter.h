// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "MySkillBase.h"
#include "Enums.h"
#include "BombAttackInterface.h"
#include "MyWindCutter.generated.h"

UCLASS()
class STATISTIC_API AMyWindCutter : public AMySkillBase, public IBombAttackInterface
{
	GENERATED_BODY()
	
public:
	AMyWindCutter();

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
    TObjectPtr<class UBoxComponent> CollisionComponent;

    // 나이아가라 파티클 시스템
    UPROPERTY(VisibleAnywhere, Category = "Particle", meta = (AllowPrivateAccess = "true"))
    TObjectPtr<class UNiagaraComponent> WindCutterNiagaraComponent;

    // 히트 효과 나이아가라 파티클 시스템
    UPROPERTY(EditAnywhere, Category = "Particle", meta = (AllowPrivateAccess = "true"))
    TObjectPtr<class UNiagaraSystem> HitEffectNiagaraSystem;

    // Projectile Movement 컴포넌트
    UPROPERTY(VisibleAnywhere, Category = "Movement", meta = (AllowPrivateAccess = "true"))
    TObjectPtr<class UProjectileMovementComponent> MovementComponent;

	UPROPERTY(EditDefaultsOnly, Category = "Settings")
    TSubclassOf<class AMyBombAttack> BombAttackClass;

private:
	float Speed = 3000.0f;
	uint8 bIsHit : 1;

public:
    virtual void Overlap(AActor* OtherActor);
    virtual void Overlap(ACharacter* OtherActor);

    virtual void MixBombAttack(EClassType MixType, unsigned short skill_id) override;
};
