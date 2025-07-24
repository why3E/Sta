// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "InteractableInterface.h"
#include "Enums.h"
#include "MyItemWorldActor.generated.h"

class UNiagaraComponent;
class UNiagaraSystem;
class USphereComponent;
class USceneComponent;
class UUserWidget;

UCLASS()
class STATISTIC_API AMyItemWorldActor : public AActor, public IInteractableInterface
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AMyItemWorldActor();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	virtual void Interact(class APlayerCharacter* InteractingPlayer) override;


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

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Item")
    EItemWorldType ItemType = EItemWorldType::HP_Flower; // 기본값(원하는 값으로)

    // 메시 배열 (블루프린트에서 타입별로 세팅)
    UPROPERTY(EditAnywhere, Category="Item")
    TMap<EItemWorldType, UStaticMesh*> ItemMeshes;
};
