// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/WidgetComponent.h"
#include "DamageWidget.h"
#include "DamagePopupActor.generated.h"

UCLASS()
class STATISTIC_API ADamagePopupActor : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	ADamagePopupActor();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	void InitDamage(float damage, EClassType Type);

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Damage")
	UWidgetComponent* widgetComp;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Damage")
	UDamageWidget* damageWidgetInstance;
};
