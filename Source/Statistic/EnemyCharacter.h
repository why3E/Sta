// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "AnimationAttackInterface.h"
#include "ReceiveDamageInterface.h" // Ensure this file defines the ReceiveDamageInterface class or struct
#include "EnemyCharacter.generated.h"

UCLASS()
class STATISTIC_API AEnemyCharacter : public ACharacter, public IReceiveDamageInterface, public IAnimationAttackInterface
{
	GENERATED_BODY()

public:	
	// Sets default values for this character's properties
	AEnemyCharacter();
private:
	bool bIsAttacking;

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	void Move(const FVector& MoveVector);

public:	
	DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnAttackEnded);

	UPROPERTY(BlueprintAssignable, Category = "Combat")
	FOnAttackEnded OnAttackEnded;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
	UAnimMontage* AttackMontage;

	UFUNCTION(BlueprintPure, Category = "Combat")
	bool GetIsAttacking() const { return bIsAttacking; }

	UFUNCTION()
	void OnAttackMontageEnded(UAnimMontage* Montage, bool bInterrupted);

	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

	void MeleeAttack();

protected:
	virtual void ReceiveSkillHit(const FSkillInfo& Info, AActor* Causer) override;

private:
    // 캐릭터의 체력
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats", Meta = (AllowPrivateAccess = "true"))
    float HP = 100.0f; // 기본 체력 값
protected:
	virtual void BaseAttackCheck() override;
};
