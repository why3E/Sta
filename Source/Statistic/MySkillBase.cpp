// Fill out your copyright notice in the Description page of Project Settings.


#include "MySkillBase.h"

// Sets default values
AMySkillBase::AMySkillBase() {
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

}

AMySkillBase::~AMySkillBase() {
	g_skills.erase(m_id);
}

void AMySkillBase::Overlap(AActor* OtherActor) {}