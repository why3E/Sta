#pragma once

#include "SESSION.h"
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

    unsigned short m_id = INVALID_OBJECT_ID;

public:
    virtual void Tick(float DeltaTime) override;

    UFUNCTION()
    void OnTorchBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);
    void Overlap();

    UFUNCTION()
	void SetMainAltar(AMyAltarMain* InAltar);

    void set_id(unsigned short id) { m_id = id; }
    unsigned short get_id() { return m_id; }

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

public:
    bool GetbIsActivated() { return bIsActivated; }
};
