// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "InputActionValue.h"
#include "CoreMinimal.h"
#include "MyCharacterBase.h"
#include "AnimationAttackInterface.h"
#include "Enums.h" // EClassType 포함
#include "MMComboActionData.h" // 데이터 에셋 헤더 포함
#include "AnimationUpdateInterface.h"
#include "MyPlayerVisualInterface.h"
#include "ImpactPointInterface.h"
#include "AnimationWeaponInterface.h"
#include "ReceiveDamageInterface.h"
#include "PlayerCharacter.generated.h"

UCLASS()
class STATISTIC_API APlayerCharacter : public AMyCharacterBase, public IAnimationAttackInterface, public IAnimationUpdateInterface, public IMyPlayerVisualInterface, public IAnimationWeaponInterface, public IImpactPointInterface
{
	GENERATED_BODY()

public:
	APlayerCharacter();

protected:
	virtual void BeginPlay() override;
	virtual void Tick(float DeltaTime);

protected:
	FORCEINLINE virtual class UCameraComponent* GetPlayerCamera() override { return Camera; }
	FORCEINLINE virtual class USpringArmComponent* GetSpringArm() override { return SpringArm; }
	FORCEINLINE virtual FVector2D GetMovementVector() override { return MovementVector; }

	// 셀카봉 역할을 해줄 컴포넌트
	UPROPERTY(VisibleAnywhere, Category = Camera, Meta = (AllowPrivateAccess = "true"))
	TObjectPtr<class USpringArmComponent> SpringArm;

	// 실제 촬영을 위한 컴포넌트
	UPROPERTY(VisibleAnywhere, Category = Camera, Meta = (AllowPrivateAccess = "true"))
	TObjectPtr<class UCameraComponent> Camera;

	FVector2D MovementVector;

	FORCEINLINE virtual class AMyWeapon* GetWeapon() override { return CurrentWeapon; }
	
protected:
	void BasicMove(const FInputActionValue& Value);
	void BasicLook(const FInputActionValue& Value);
	void StartJump();
    void StopJump();
	void DashStart();
	void DashEnd();
	void BasicAttack();
	void SkillAttack();
	void QSkill();
	void ESkill();
	void ChangeClassTest();

	UPROPERTY(VisibleAnywhere, Category = Input, Meta = (AllowPrivateAccess = "true"))
	TObjectPtr<class UInputMappingContext> IMC_Basic;

	UPROPERTY(VisibleAnywhere, Category = Input, Meta = (AllowPrivateAccess = "true"))
	TObjectPtr<class UInputAction> IA_BasicMove;

	UPROPERTY(VisibleAnywhere, Category = Input, Meta = (AllowPrivateAccess = "true"))
	TObjectPtr<class UInputAction> IA_BasicLook;

	UPROPERTY(VisibleAnywhere, Category = Input, Meta = (AllowPrivateAccess = "true"))
    TObjectPtr<class UInputAction> IA_BasicJump;

	UPROPERTY(VisibleAnywhere, Category = Input, Meta = (AllowPrivateAccess = "true"))
	TObjectPtr<class UInputAction> IA_Dash;

	UPROPERTY(VisibleAnywhere, Category = Input, Meta = (AllowPrivateAccess = "true"))
	TObjectPtr<class UInputAction> IA_BasicAttack;

	UPROPERTY(VisibleAnywhere, Category = Input, Meta = (AllowPrivateAccess = "true"))
	TObjectPtr<class UInputAction> IA_QSkill;

	UPROPERTY(VisibleAnywhere, Category = Input, Meta = (AllowPrivateAccess = "true"))
	TObjectPtr<class UInputAction> IA_ESkill;

	UPROPERTY(VisibleAnywhere, Category = Input, Meta = (AllowPrivateAccess = "true"))
	TObjectPtr<class UInputAction> IA_ChangeClass;


protected:
	uint8 bIsDash : 1;
public:
	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

// Combo
// Montage
protected:
	UPROPERTY(EditAnywhere, Category = Montage, Meta = (AllowPrivateAccess = "true"))
	TObjectPtr<class UAnimMontage> StoneComboMontage;
	UPROPERTY(EditAnywhere, Category = Montage, Meta = (AllowPrivateAccess = "true"))
	TObjectPtr<class UAnimMontage> WindComboMontage;
	UPROPERTY(EditAnywhere, Category = Montage, Meta = (AllowPrivateAccess = "true"))
	TObjectPtr<class UAnimMontage> FireComboMontage;

// Combo
protected:
	UPROPERTY(EditAnywhere, Category = ComboData, Meta = (AllowPrivateAccess = "true"))
	TObjectPtr<class UMMComboActionData> StoneComboData;
	UPROPERTY(EditAnywhere, Category = ComboData, Meta = (AllowPrivateAccess = "true"))
	TObjectPtr<class UMMComboActionData> WindComboData;
	UPROPERTY(EditAnywhere, Category = ComboData, Meta = (AllowPrivateAccess = "true"))
	TObjectPtr<class UMMComboActionData> FireComboData;

protected:
	void ComboStart();
	void ComboEnd(class UAnimMontage* Montage, bool IsEnded);
	void ComboCheck();
	void SetComboTimer();

	// 콤보에 사용될 타이머 변수
	FTimerHandle ComboTimerHandle;
	// 현재 콤보 진행 수
	int32 CurrentComboCount = 0;
	// 콤보 입력 판별
	uint8 bHasComboInput : 1;

	UPROPERTY(BlueprintReadOnly, Category = "Movement")
	uint8 CheckBackMove = 0;

protected:
	virtual void BaseAttackCheck() override;

	EClassType ClassType;

	// 클래스 변경 함수
	void ChangeClass(EClassType NewClassType);

public:
    // 블루프린트에서 접근 가능한 변수
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Custom")
    uint8 CheckAnimBone : 1;

private:
	// 캐싱된 현재 몽타주와 데이터
	UAnimMontage* CurrentMontage;
	UMMComboActionData* CurrentComboData;
	FString CurrentMontageSectionName; // 섹션 이름을 저장하는 변수

	// 캐싱된 데이터를 업데이트하는 함수 
	void UpdateCachedData();

	// MMPlayerCharacter Header
	void EquipWeapon(class AMyWeapon* Weapon);

	// TEST
	UPROPERTY(EditAnywhere, Category = Weapon, meta = (AllowPrivateAccess = "true"))
	TSubclassOf<class AMyWeapon> WeaponClass;

	UPROPERTY(VisibleAnywhere, Category = Weapon, meta = (AllowPrivateAccess = "true"))
	TObjectPtr<class AMyWeapon> CurrentWeapon;

protected:
	FORCEINLINE virtual EClassType GetClassType() override { return ClassType; };

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon")
    TSubclassOf<AMyWeapon> FireWeaponBP;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon")
    TSubclassOf<AMyWeapon> WindWeaponBP;

private:
    bool bIsDrawingCircle = false; // 원을 그리고 있는지 여부
    FVector CurrentImpactPoint;   // 현재 충돌 지점
	FRotator CurrentImpactRot;     // 현재 충돌 회전
    FTimerHandle CircleUpdateTimerHandle; // 원 업데이트를 위한 타이머 핸들
	void UpdateCircle();

protected:
	FORCEINLINE virtual FVector GetCurrentImpactPoint() override { return CurrentImpactPoint; }
	FORCEINLINE virtual FRotator GetCurrentImpactRot() override { return CurrentImpactRot; }
	FORCEINLINE virtual FVector GetFireLocation() override { return FireLocation; }

private:
	FVector FireLocation;
	float TraceDistance = 2000.0f; // 타겟 거리

protected:
	// 타겟 위치를 구하기 위한 함수
	void GetFireTargetLocation();

private:
	char m_id;
	float m_yaw;
	FVector m_velocity;
	char m_hp;
	char m_current_element;

	FVector m_skill_velocity;
	FVector m_skill_location;

	bool m_is_player;
	bool m_was_moving;

public:
	void do_send(void* buff);

	char get_id() { return m_id; }
	FVector get_skill_velocity() { return  m_skill_velocity; }
	FVector get_skill_location() { return  m_skill_location; }
	bool get_is_player() { return m_is_player; }

	void set_id(char id) { m_id = id; }
	void set_velocity(float x, float y, float z) { m_velocity.X = x; m_velocity.Y = y; m_velocity.Z = z; }
	void set_skill_velocity(float x, float y, float z) { m_skill_velocity.X = x; m_skill_velocity.Y = y; m_skill_velocity.Z = z; }
	void set_skill_location(float x, float y, float z) { m_skill_location.X = x; m_skill_location.Y = y; m_skill_location.Z = z; }
	void set_is_player(bool is_player) { m_is_player = is_player; }

	void rotate(float yaw);
	void use_skill(char skill_type, FVector v);
};