// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "MyCharacterBase.h"
#include "InteractableInterface.h"
#include "MyNPC.generated.h"

class USphereComponent;
class UWidgetComponent;
class APlayerController;
class APlayerCharacter;
class ACineCameraActor;

UCLASS()
class STATISTIC_API AMyNPC : public AMyCharacterBase, public IInteractableInterface
{
	GENERATED_BODY()
public:
	AMyNPC();

	virtual void BeginPlay() override;
	virtual void Tick(float DeltaTime) override;
	void StartInteractionCamera();
	
public:
    virtual void Interact(APlayerCharacter* InteractingPlayer) override;

protected:
	UPROPERTY(EditAnywhere)
	class USphereComponent* InteractionSphere;

	UPROPERTY(EditAnywhere)
	class UWidgetComponent* WidgetInteractionComponent;

	UFUNCTION()
	void OnBeginOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);
	UFUNCTION()
	void OnEndOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);

	UPROPERTY()
	ACineCameraActor* npcInteractionCamera;

	UPROPERTY(EditAnywhere, Category = "Sequence")
	FName npcCameraTagName;

private:
    APlayerCharacter* cachedPlayer = nullptr;
    APlayerController* cachedController = nullptr;
};
