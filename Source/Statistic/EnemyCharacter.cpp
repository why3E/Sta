#include "EnemyCharacter.h"
#include "Components/CapsuleComponent.h"
#include "GameFramework/CharacterMovementComponent.h"

// Sets default values
AEnemyCharacter::AEnemyCharacter()
{
    // Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
    PrimaryActorTick.bCanEverTick = false;

    GetCapsuleComponent()->InitCapsuleSize(42.f, 50.0f);

    GetCharacterMovement()->bOrientRotationToMovement = true; // Character moves in the direction of input...
    GetCharacterMovement()->RotationRate = FRotator(0.0f, 360.0f, 0.0f); // ...at this rotation rate

    GetCharacterMovement()->JumpZVelocity = 700.f;
    GetCharacterMovement()->AirControl = 0.35f;
    GetCharacterMovement()->MaxWalkSpeed = 500.f;
    GetCharacterMovement()->MinAnalogWalkSpeed = 20.f;
    GetCharacterMovement()->BrakingDecelerationWalking = 2000.f;
    GetCharacterMovement()->BrakingDecelerationFalling = 1500.f;
    
    bUseControllerRotationYaw = false; // 컨트롤러의 Yaw 회전을 사용하지 않음
    GetCharacterMovement()->bUseControllerDesiredRotation = false; // 컨트롤러가 원하는 회전을 사용하지 않음
}

// Called when the game starts or when spawned
void AEnemyCharacter::BeginPlay()
{
    Super::BeginPlay();
    bIsAttacking = false;
}


// Called every frame
void AEnemyCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

// Called to bind functionality to input
void AEnemyCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

}

void AEnemyCharacter::OnAttackMontageEnded(UAnimMontage* Montage, bool bInterrupted)
{
    if (Montage == AttackMontage)
    {
        bIsAttacking = false;
        OnAttackEnded.Broadcast();
    }
}

void AEnemyCharacter::MeleeAttack()
{
    if (bIsAttacking)
    {
        return;
    }

    if (AttackMontage)
    {
        // Get the character's animation instance
        UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();
        if (AnimInstance)
        {
            // Play the attack montage
            float MontageDuration = AnimInstance->Montage_Play(AttackMontage, 1.0f);
            if (MontageDuration > 0.f)
            {
                // Set attacking flag to true
                bIsAttacking = true;

                // Bind function to Montage ended event
                FOnMontageEnded MontageEndedDelegate;
                MontageEndedDelegate.BindUObject(this, &AEnemyCharacter::OnAttackMontageEnded);
                AnimInstance->Montage_SetEndDelegate(MontageEndedDelegate, AttackMontage);
            }
        }
    }
}

