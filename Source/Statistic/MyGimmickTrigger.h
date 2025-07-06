// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "InteractableInterface.h"
#include "MyGimmickTrigger.generated.h"

class UBoxComponent;
class USceneComponent;
class APlayerCharacter;
class UUserWidget;
class APlayerController;

UCLASS()
class STATISTIC_API AMyGimmickTrigger : public AActor , public IInteractableInterface
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AMyGimmickTrigger();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	UBoxComponent* boxCollision;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	USceneComponent* sceneComp;

    UPROPERTY(EditAnywhere, Category="UI")
    TSubclassOf<UUserWidget> interactionWidgetClass;
	
	virtual void Interact(APlayerCharacter* InteractingPlayer) override;

protected:
	UPROPERTY()
	UUserWidget* interactionWidgetInstance;
	
    APlayerCharacter* cachedPlayer = nullptr;
    APlayerController* cachedController = nullptr;
	
protected:
    UFUNCTION()
    void OnBeginOverlapCollision(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
        UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

    UFUNCTION()
    void OnEndOverlapCollision(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
    UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);


};
