// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "SESSION.h"
#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "MyBombActor.generated.h"

class USceneComponent;
class UBoxComponent;
class AMyBoomsGimmickTrigger;
class AMySkillBase;

UCLASS()
class STATISTIC_API AMyBombActor : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AMyBombActor();
	void SetTriggerOwner(AMyBoomsGimmickTrigger* Trigger);  // 트리거 등록

protected:
    virtual void BeginPlay() override;
    virtual void Destroyed() override;  // 파괴될 때 호출됨

    UPROPERTY(VisibleAnywhere)
    USceneComponent* SceneComp;

    UPROPERTY(VisibleAnywhere)
    UBoxComponent* BoxComp;

	unsigned short m_id = INVALID_OBJECT_ID;

private:
    UPROPERTY()
    AMyBoomsGimmickTrigger* TriggerOwner;
	
	UFUNCTION()
	void OnBeginOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;
	void Overlap();

	void set_id(unsigned short id) { m_id = id; }
	unsigned short get_id() { return m_id; }
};
