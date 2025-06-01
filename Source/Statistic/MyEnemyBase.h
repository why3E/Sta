// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "MyEnemyBase.generated.h"

UCLASS()
class STATISTIC_API AMyEnemyBase : public ACharacter {
	GENERATED_BODY()

public:
	// Sets default values for this character's properties
	AMyEnemyBase();

	// ID
	unsigned short m_id;
	FVector m_target_location;

	void set_id(unsigned short id) { m_id = id; }
	void set_target_location(FVector target_location) { m_target_location = target_location; }

	unsigned short get_id() { return m_id; }
	FVector get_target_location() { return m_target_location; }

	// HP
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats", Meta = (AllowPrivateAccess = "true"))
	float HP = 100.0f;
	float MaxHP = 100.0f;

	void SetHP(float hp) { HP = hp; }
	float GetHP() { return HP; }

	// Heal
	FTimerHandle HealTimerHandle;

	void StartHeal();
	void StopHeal();
	void HealTick();
	void Heal(float HealAmount);

	// Die
	FTimerHandle RespawnTimerHandle;

	virtual void Die();
	virtual void Reset();
	virtual void Respawn();
	virtual void Respawn(FVector respawn_location);

	// Collision
	virtual void Overlap(char skill_type, FVector skill_location);
};
