// Fill out your copyright notice in the Description page of Project Settings.


#include "MySkillBase.h"

// Sets default values
AMySkillBase::AMySkillBase() {
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
}

AMySkillBase::~AMySkillBase() {
    if (g_c_skills.count(m_id)) {
        g_c_skills.erase(m_id);
    }
}

void AMySkillBase::Overlap(char skill_type) {

}

void AMySkillBase::Overlap(unsigned short object_id, bool collision_start) {

}
