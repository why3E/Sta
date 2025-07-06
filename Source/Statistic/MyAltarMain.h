#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "MyAltarMain.generated.h"

class AMyAltarTorch;
class UStaticMeshComponent;

UCLASS()
class STATISTIC_API AMyAltarMain : public AActor
{
    GENERATED_BODY()

public:
    AMyAltarMain();

protected:
    virtual void BeginPlay() override;

public:
    virtual void Tick(float DeltaTime) override;

    void NotifyTorchActivated();

protected:
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
    USceneComponent* SceneRoot;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
    UStaticMeshComponent* AltarMesh1;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
    UStaticMeshComponent* AltarMesh2;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
    UStaticMeshComponent* AltarMesh3;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
    UStaticMeshComponent* AltarMesh4;

    UPROPERTY(EditAnywhere, Category = "Torch")
    TSubclassOf<AActor> ChestClass;

    UPROPERTY(EditAnywhere, Category = "Torch")
    TArray<AActor*> TorchesSpawnTargets;

	UPROPERTY(EditAnywhere, Category = "Torch")
	TSubclassOf<AMyAltarTorch> TorchClass;


private:
    int32 ActivatedTorchCount = 0;

    void UpdateAltarState();
};
