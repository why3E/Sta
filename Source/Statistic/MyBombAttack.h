#pragma once

#include "CoreMinimal.h"
#include "MySkillBase.h"
#include "Enums.h"
#include "MyBombAttack.generated.h"

UCLASS()
class STATISTIC_API AMyBombAttack : public AMySkillBase
{
    GENERATED_BODY()

public:
    AMyBombAttack();

protected:
    virtual void BeginPlay() override;
    virtual void PostInitializeComponents() override;
    virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

public:
    virtual void Tick(float DeltaTime) override;
    void SpawnBombAttack(FVector ImpactPoint, EClassType MixType);

    virtual void Overlap(AActor* OtherActor);

protected:
    UPROPERTY(VisibleAnywhere, Category = "Collision")
    TObjectPtr<class UStaticMeshComponent> CollisionMesh;

    UPROPERTY(VisibleAnywhere, Category = "Effects")
    TObjectPtr<class UNiagaraComponent> MixBombAttackNiagaraComponent;

    UPROPERTY(EditAnywhere, Category = "Effects")
    TObjectPtr<class UNiagaraSystem> MixBombAttackEffect;

    UPROPERTY(EditAnywhere, Category = "Effects")
    TObjectPtr<class UNiagaraSystem> FireEffect;

    UPROPERTY(EditAnywhere, Category = "Damage")
    float SkillDamage = 20.0f;

    UPROPERTY(EditAnywhere, Category = "Damage")
    EClassType SkillElement = EClassType::CT_Wind;

    // 타이머 핸들 추가
    FTimerHandle CheckOverlapTimerHandle;

    UFUNCTION()
    void OnBeginOverlap(UPrimitiveComponent* OverlappedComp,
        AActor* OtherActor,
        UPrimitiveComponent* OtherComp,
        int32 OtherBodyIndex,
        bool bFromSweep,
        const FHitResult& SweepResult);

private:
    void CheckOverlappingActors();
};
