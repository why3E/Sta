// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "SESSION.h"
#include "CoreMinimal.h"
#include "AIController.h"
#include "GameFramework/Character.h"
#include "BehaviorTree/BehaviorTree.h"
#include "MyEnemyBase.generated.h"

UCLASS()
class STATISTIC_API AMyEnemyBase : public ACharacter {
	GENERATED_BODY()

public:
	// Sets default values for this character's properties
	AMyEnemyBase();

	// ID
	unsigned short m_id;
	MonsterType m_type;
	std::atomic<bool> m_is_active = true;

	FVector m_target_location;

	float m_view_radius;
	float m_track_radius;
	float m_wander_radius;

	void set_id(unsigned short id) { m_id = id; }
	void set_type(MonsterType type) { m_type = type; }
	void set_target_location(FVector target_location) { m_target_location = target_location; }

	unsigned short get_id() { return m_id; }
	MonsterType get_type() { return m_type; }
	FVector get_target_location() { return m_target_location; }

	// Combat
	unsigned short m_skill_id;
	
	float m_attack_radius;

	void set_skill_id(unsigned short skill_id) { m_skill_id = skill_id; }
	void set_attack_radius(float attack_radius) { m_attack_radius = attack_radius; }

	unsigned short get_skill_id() { return m_skill_id; }
	virtual float get_attack_radius() { return m_attack_radius; }

	virtual void start_attack(AttackType attack_type);
	virtual void start_attack(AttackType attack_type, FVector attack_location);

	void sleep() { 
		bool expected = true;

		if (std::atomic_compare_exchange_strong(&m_is_active, &expected, false)) {
			AAIController* AICon = Cast<AAIController>(GetController());

			if (AICon) {
				AICon->StopMovement();

				if (AICon->BrainComponent) {
					AICon->BrainComponent->StopLogic(TEXT(""));
					UE_LOG(LogTemp, Error, TEXT("Monster %d is Now Sleeping"), m_id);
				}
			}
		}
	}

	void wake_up() { 
		bool expected = false;

		if (std::atomic_compare_exchange_strong(&m_is_active, &expected, true)) {
			AAIController* AICon = Cast<AAIController>(GetController());
			UBehaviorTree* BTAsset = LoadObject<UBehaviorTree>(nullptr, TEXT("/Game/Monster/Slime/AI/BT_EnemyAI.BT_EnemyAI"));

			AICon->RunBehaviorTree(BTAsset);
			UE_LOG(LogTemp, Error, TEXT("Monster %d is Now Active"), m_id);
		}
	}

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
