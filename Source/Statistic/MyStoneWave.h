#pragma once

#include "CoreMinimal.h"
#include "MySkillBase.h"
#include "MyStoneWave.generated.h"

UCLASS()
class STATISTIC_API AMyStoneWave : public AMySkillBase
{
	GENERATED_BODY()
	
public:
	AMyStoneWave();

protected:
	virtual void BeginPlay() override;
	virtual void PostInitializeComponents() override;

public:	
	virtual void Tick(float DeltaTime) override;
	void Fire(FVector TargetLocation);
	void ActivateNiagara();

protected:
	UFUNCTION()
	void OnBeginOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

	UFUNCTION()
	void OnNiagaraFinished(class UNiagaraComponent* PSystem);

	void StartDestroyTimer();
	void DestroySelf();

	UPROPERTY(VisibleAnywhere, Category = "Collision")
	TObjectPtr<class UBoxComponent> CollisionComponent;

	UPROPERTY(VisibleAnywhere, Category = "Particle")
	TObjectPtr<class UNiagaraComponent> StoneWaveNiagaraComponent;

	UPROPERTY(VisibleAnywhere, Category = "Movement")
	TObjectPtr<class UProjectileMovementComponent> MovementComponent;

private:
	float Speed = 2000.0f;
	uint8 bIsHit : 1;
	FTimerHandle DestroyTimerHandle;

public:
	virtual void Overlap(char skill_type) override;
	virtual void Overlap(unsigned short object_id, bool collision_start) override;
};
