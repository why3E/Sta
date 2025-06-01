// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "InteractableInterface.h"
#include "PlayerCharacter.h"
#include "Blueprint/UserWidget.h"
#include "MyItemDropActor.generated.h"

class UNiagaraComponent;
class UNiagaraSystem;
class USphereComponent;
class USceneComponent;

UCLASS()
class STATISTIC_API AMyItemDropActor : public AActor, public IInteractableInterface
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AMyItemDropActor();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	virtual void Interact(class APlayerCharacter* InteractingPlayer) override;

	void SpawnItem(const FVector& StartLocation);

protected:
    UPROPERTY(VisibleAnywhere)
    UStaticMeshComponent* ItemMesh;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
    UNiagaraComponent* ItemEffectComponent;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    UNiagaraSystem* ItemEffectSystem;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
    USphereComponent* ItemCollision;


protected:
    UFUNCTION()
    void OnBeginOverlapCollision(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
        UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

    UFUNCTION()
    void OnEndOverlapCollision(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
    UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TSubclassOf<UUserWidget> interactionWidgetClass;

    UUserWidget* interactionWidgetInstance = nullptr;
};
