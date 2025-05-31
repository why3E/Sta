#pragma once

#include "CoreMinimal.h"
#include "MySkillBase.h"
#include "MyWindLaser.generated.h"

class UNiagaraComponent;
class UCapsuleComponent;
UCLASS()
class STATISTIC_API AMyWindLaser : public AMySkillBase
{
    GENERATED_BODY()

public:
    AMyWindLaser();

    void SpawnChargingLaser(); // 차징만
    void SpawnFiringLaser();   // 발사만

protected:
    virtual void BeginPlay() override;
    virtual void PostInitializeComponents() override;
    virtual void EndPlay(EEndPlayReason::Type EndPlayReason) override;
    virtual void Tick(float DeltaTime) override;

    UFUNCTION()
    void OnChargingFinished(UNiagaraComponent* PSystem);

    UFUNCTION()
    void OnFiringFinished(UNiagaraComponent* PSystem);

    UFUNCTION()
    void OnOverlapBegin(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
                        UPrimitiveComponent* OtherComp, int32 OtherBodyIndex,
                        bool bFromSweep, const FHitResult& SweepResult);

protected:
    UPROPERTY(VisibleAnywhere)
    USceneComponent* RootScene;

    UPROPERTY(VisibleAnywhere)
    UNiagaraComponent* ChargingEffect;

    UPROPERTY(VisibleAnywhere)
    UNiagaraComponent* FiringEffect;

    UPROPERTY(VisibleAnywhere)
    UCapsuleComponent* CollisionCapsule;

    bool bIsFiring;
};
