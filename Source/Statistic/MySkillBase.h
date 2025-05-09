// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "SESSION.h"

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "MySkillBase.generated.h"

UCLASS()
class STATISTIC_API AMySkillBase : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AMySkillBase();
	~AMySkillBase();

	virtual void Overlap();

	UPROPERTY()
	uint16 m_id;

	void SetID(unsigned short skill_id) { m_id = skill_id; }
};
