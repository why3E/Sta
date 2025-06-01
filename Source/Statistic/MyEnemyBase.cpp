// Fill out your copyright notice in the Description page of Project Settings.

#include "MyEnemyBase.h"
#include "SESSION.h"

// Sets default values
AMyEnemyBase::AMyEnemyBase() {
 	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
}

void AMyEnemyBase::start_attack(AttackType attack_type) {

}

void AMyEnemyBase::start_attack(AttackType attack_type, FVector attack_location) {

}

void AMyEnemyBase::StartHeal() {
    if (!GetWorldTimerManager().IsTimerActive(HealTimerHandle)) {
        if (HP < MaxHP) {
            GetWorldTimerManager().SetTimer(HealTimerHandle, this, &AMyEnemyBase::HealTick, 0.1f, true);
        }
    }
}

void AMyEnemyBase::StopHeal() {
    GetWorldTimerManager().ClearTimer(HealTimerHandle);
}

void AMyEnemyBase::HealTick() {
    Heal(10.0f);

    MonsterEvent monster_event = HealEvent(m_id, 10.0f);
    std::lock_guard<std::mutex> lock(g_s_monster_events_l);
    g_s_monster_events.push(monster_event);
}

void AMyEnemyBase::Heal(float HealAmount) {
    HP += HealAmount;

    if (HP > MaxHP) {
        HP = MaxHP;
    }
}

void AMyEnemyBase::Die() {

}

void AMyEnemyBase::Reset() {

}

void AMyEnemyBase::Respawn() {

}

void AMyEnemyBase::Respawn(FVector respawn_location) {

}

void AMyEnemyBase::Overlap(char skill_type, FVector skill_location) {

}
