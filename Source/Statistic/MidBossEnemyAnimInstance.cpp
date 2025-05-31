// Fill out your copyright notice in the Description page of Project Settings.


#include "MidBossEnemyAnimInstance.h"
#include "MidBossEnemyCharacter.h"

void UMidBossEnemyAnimInstance::NativeUpdateAnimation(float DeltaSeconds)
{
    Super::NativeUpdateAnimation(DeltaSeconds);

    auto* WoodMonster = Cast<AMidBossEnemyCharacter>(TryGetPawnOwner());
    if (WoodMonster)
    {
        FVector velocity = WoodMonster->GetVelocity();
        FVector forward = WoodMonster->GetActorForwardVector();
        speedX = FVector::DotProduct(velocity, forward);

        FVector right = WoodMonster->GetActorRightVector();
        speedY = FVector::DotProduct(velocity, right);
    }
}