#pragma once

#include "CoreMinimal.h"
#include "MySkillBase.h"
#include "Sound/SoundBase.h"
#include "NiagaraComponent.h"
#include "NiagaraSystem.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "MyIceArrow.generated.h"

UCLASS()
class STATISTIC_API AMyIceArrow : public AMySkillBase
{
    GENERATED_BODY()
    
public:	
    AMyIceArrow();

protected:
    virtual void BeginPlay() override;
    virtual void PostInitializeComponents() override;

public:	
    virtual void Tick(float DeltaTime) override;
    void Fire(FVector TargetLocation);

    void ActivateNiagara();
    
    virtual void Overlap(AActor* OtherActor);
    virtual void Overlap(ACharacter* OtherActor);

protected:
    // 콜리전 처리 함수
    UFUNCTION()
    void OnBeginOverlap(class UPrimitiveComponent* OverlappedComp, class AActor* OtherActor, class UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

    // 콜리전 컴포넌트
    UPROPERTY(VisibleAnywhere, Category = "Collision", meta = (AllowPrivateAccess = "true"))
    TObjectPtr<class USphereComponent> CollisionComponent;

    // 나이아가라 파티클 시스템
    UPROPERTY(VisibleAnywhere, Category = "Particle", meta = (AllowPrivateAccess = "true"))
    TObjectPtr<class UNiagaraComponent> IceArrowNiagaraComponent;

    // 히트 효과 나이아가라 파티클 시스템
    UPROPERTY(EditAnywhere, Category = "Particle", meta = (AllowPrivateAccess = "true"))
    TObjectPtr<class UNiagaraSystem> HitEffectNiagaraSystem;

    // Projectile Movement 컴포넌트
    UPROPERTY(VisibleAnywhere, Category = "Movement", meta = (AllowPrivateAccess = "true"))
    TObjectPtr<class UProjectileMovementComponent> MovementComponent;

private:
    float Speed = 3000.0f;
    uint8 bIsHit : 1;
protected:
    // 효과음
    UPROPERTY(EditAnywhere, Category = "Sound", meta = (AllowPrivateAccess = "true"))
    TObjectPtr<class USoundBase> IceArrowHitShootSound;
};