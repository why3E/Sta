#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "MyRunPointActor.generated.h"

class USceneComponent;
class USphereComponent;
class AMyRunGimmickTrigger;
class AMySkillBase;

UCLASS()
class STATISTIC_API AMyRunPointActor : public AActor
{
	GENERATED_BODY()
	
public:	
	AMyRunPointActor();
	
	void SetTriggerOwner(AMyRunGimmickTrigger* Trigger);  // 트리거 등록

protected:
	virtual void BeginPlay() override;
	virtual void Destroyed() override;

	UPROPERTY(VisibleAnywhere)
	USceneComponent* SceneComp;

	UPROPERTY(VisibleAnywhere)
	USphereComponent* SphereComp;

private:
	UPROPERTY()
	AMyRunGimmickTrigger* TriggerOwner;

	UFUNCTION()
	void OnBeginOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
	                    UPrimitiveComponent* OtherComp, int32 OtherBodyIndex,
	                    bool bFromSweep, const FHitResult& SweepResult);

public:	
	virtual void Tick(float DeltaTime) override;
};
