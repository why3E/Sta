// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "SESSION.h"
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

	float m_view_radius;
	float m_track_radius;
	float m_wander_radius;

	void set_id(unsigned short id) { m_id = id; }
	void set_target_location(FVector target_location) { m_target_location = target_location; }

	unsigned short get_id() { return m_id; }
	FVector get_target_location() { return m_target_location; }

	// Combat
	unsigned short m_skill_id;
	
	float m_attack_radius;

	void set_skill_id(unsigned short skill_id) { m_skill_id = skill_id; }
	void set_attack_radius(float attack_radius) { m_attack_radius = attack_radius; }

	unsigned short get_skill_id() { return m_skill_id; }
	float get_attack_radius() { return m_attack_radius; }

	virtual void start_attack(AttackType attack_type);
	virtual void start_attack(AttackType attack_type, FVector attack_location);

	// HP
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats", Meta = (AllowPrivateAccess = "true"))
	float HP;
	float MaxHP;

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
