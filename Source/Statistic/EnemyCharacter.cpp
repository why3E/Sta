#include "EnemyCharacter.h"
#include "Components/CapsuleComponent.h"
#include "Debug/DebugDrawService.h"
#include "GameFramework/CharacterMovementComponent.h"

// Sets default values
AEnemyCharacter::AEnemyCharacter()
{
    // Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
    PrimaryActorTick.bCanEverTick = false;

    GetCapsuleComponent()->InitCapsuleSize(42.f, 50.0f);

    GetCharacterMovement()->bOrientRotationToMovement = true; // Character moves in the direction of input...
    GetCharacterMovement()->RotationRate = FRotator(0.0f, 720.0f, 0.0f); // ...at this rotation rate

    GetCharacterMovement()->JumpZVelocity = 700.f;
    GetCharacterMovement()->AirControl = 0.35f;
    GetCharacterMovement()->MaxWalkSpeed = 500.f;
    GetCharacterMovement()->MinAnalogWalkSpeed = 20.f;
    GetCharacterMovement()->BrakingDecelerationWalking = 2000.f;
    GetCharacterMovement()->BrakingDecelerationFalling = 1500.f;
    
    GetCharacterMovement()->bOrientRotationToMovement = false;
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

void AEnemyCharacter::ReceiveSkillHit(const FSkillInfo& Info, AActor* Causer)
{
    //float Resistance = GetResistanceAgainst(Info.Element);
    float FinalDamage = Info.Damage;// * (1.0f - Resistance);
    HP = HP - FinalDamage;

    if (Info.StunTime > 0.0f)
    {
        //Stun(Info.StunTime);
    }
	
	UE_LOG(LogTemp, Warning, TEXT("Receive Skill Hit! Damage: %f"), FinalDamage);
	UE_LOG(LogTemp, Warning, TEXT("Current HP: %f"), HP);
}

void AEnemyCharacter::BaseAttackCheck()
{
	// 충돌 결과를 반환하기 위한 배열
	TArray<FHitResult> OutHitResults;

	// 공격 반경
	float AttackRange = 100.0f;
	// 공격 체크를 위한 구체의 반지름
	float AttackRadius = 50.0f;

	// 충돌 탐지를 위한 시작 지점 (플레이어 현재 위치 + 전방 방향 플레이어의 CapsuleComponent의 반지름 거리)
	FVector Start = GetActorLocation() + (GetActorForwardVector() * GetCapsuleComponent()->GetScaledCapsuleRadius());
	// 충돌 탐지 종료 지점 (시작지점 + 전방 방향의 공격 거리)
	FVector End = Start + (GetActorForwardVector() * AttackRange);
	// 파라미터 설정하기 (트레이스 태그 : Attack, 복잡한 충돌 처리 : false, 무시할 액터 : this) 
	FCollisionQueryParams Params(SCENE_QUERY_STAT(Attack), false, this);

	bool bHasHit = GetWorld()->SweepMultiByChannel(
		OutHitResults,
		Start,
		End,
		FQuat::Identity,
		ECC_GameTraceChannel2,
		FCollisionShape::MakeSphere(AttackRadius),
		Params
	);

	if (bHasHit)
	{
		// TODO : 데미지 전달
	}

	// Capsule 모양의 디버깅 체크
	FVector CapsuleOrigin = Start + (End - Start) * 0.5f;
	float CapsuleHalfHeight = AttackRange * 0.5f;
	FColor DrawColor = bHasHit ? FColor::Green : FColor::Red;

	DrawDebugCapsule(GetWorld(), CapsuleOrigin, CapsuleHalfHeight, AttackRadius, FRotationMatrix::MakeFromZ(GetActorForwardVector()).ToQuat(), DrawColor, false, 3.0f);
}