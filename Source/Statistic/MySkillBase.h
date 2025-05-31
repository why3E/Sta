// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "Enums.h"
#include "SESSION.h"

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "MySkillBase.generated.h"

UCLASS()
class STATISTIC_API AMySkillBase : public AActor
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, Category = "Damage")
	float Damage = 10.0f;

	UPROPERTY(EditAnywhere, Category = "Damage")
	EClassType Element = EClassType::CT_Wind;
	
public:	
	// Sets default values for this actor's properties
	AMySkillBase();
	~AMySkillBase();

	virtual void Overlap(char skill_type);
	virtual void Overlap(unsigned short object_id, bool collision_start = true);

	UPROPERTY()
	uint16 m_id;
	char m_type;

	unsigned short GetId() { return m_id; }
	char GetType() { return m_type; }
	float GetDamage() { return Damage; }
	EClassType GetElement() { return Element; }

	void SetID(unsigned short skill_id) { m_id = skill_id; }
	void SetType(char skill_type) { m_type = skill_type; }
	void SetDamage(float damage) { Damage = damage; }
	void SetElement(EClassType element) { Element = element; }
};
