// Fill out your copyright notice in the Description page of Project Settings.


#include "slimeAnimInstance.h"
#include "EnemyCharacter.h"
#include "AnimationAttackInterface.h"
#include "AnimationUpdateInterface.h"
#include "GameFramework/CharacterMovementComponent.h"

void UslimeAnimInstance::NativeUpdateAnimation(float DeltaSeconds)
{
    Super::NativeUpdateAnimation(DeltaSeconds);

    auto* slime = Cast<AEnemyCharacter>(TryGetPawnOwner());
    if (slime)
    {
        FVector velocity = slime->GetVelocity();
        FVector forward = slime->GetActorForwardVector();
        speedX = FVector::DotProduct(velocity, forward);

        FVector right = slime->GetActorRightVector();
        speedY = FVector::DotProduct(velocity, right);
    }
}