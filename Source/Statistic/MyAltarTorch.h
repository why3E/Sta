#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "MyAltarTorch.generated.h"

class UCapsuleComponent;
class UNiagaraComponent;
class AMyAltarMain;
class AMyFireSkill;
class AMyFireBall;

UCLASS()
class STATISTIC_API AMyAltarTorch : public AActor
{
    GENERATED_BODY()

public:
    AMyAltarTorch();

protected:
    virtual void BeginPlay() override;

public:
    virtual void Tick(float DeltaTime) override;

    UFUNCTION()
    void OnTorchBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
        UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

    UFUNCTION()
	void SetMainAltar(AMyAltarMain* InAltar);

protected:
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
    USceneComponent* SceneRoot;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
    UCapsuleComponent* TorchCollision;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
    UNiagaraComponent* TorchEffect;

    UPROPERTY()
    AMyAltarMain* AltarOwner;

    bool bIsActivated = false;
};
