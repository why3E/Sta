// Fill out your copyright notice in the Description page of Project Settings.
#include "PlayerAnim.h"
#include "PlayerCharacter.h"
#include "AnimationUpdateInterface.h"
#include "GameFramework/CharacterMovementComponent.h"
void UPlayerAnim::NativeUpdateAnimation(float DeltaSeconds)
{
    Super::NativeUpdateAnimation(DeltaSeconds);

    auto* player = Cast<APlayerCharacter>(TryGetPawnOwner());
    if(player)
	{
        FVector velocity = player->GetVelocity();
    	FVector forward = player->GetActorForwardVector();
    	speedX = FVector::DotProduct(velocity, forward);

    	FVector right = player->GetActorRightVector();
    	speedY = FVector::DotProduct(velocity, right);

        auto movement = player->GetCharacterMovement();
        isInAir = movement->IsFalling();
    }
    IAnimationUpdateInterface* Player = Cast<IAnimationUpdateInterface>(TryGetPawnOwner());
	if (Player)
	{
		ClassType = Player->GetClassType();
	}
}