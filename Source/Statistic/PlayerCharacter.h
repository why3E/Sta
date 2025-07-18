// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "Enums.h"
#include "SESSION.h"

#include "InputActionValue.h"
#include "CoreMinimal.h"
#include "MyCharacterBase.h"
#include "Enums.h" // EClassType 포함
#include "MMComboActionData.h" // 데이터 에셋 헤더 포함
#include "AnimationUpdateInterface.h"
#include "MyPlayerVisualInterface.h"
#include "ImpactPointInterface.h"
#include "AnimationWeaponInterface.h"
#include "ReceiveDamageInterface.h"

#include "Components/SceneCaptureComponent2D.h"
#include "Engine/TextureRenderTarget2D.h"
#include "PaperSpriteComponent.h"

#include "PlayerCharacter.generated.h"

UCLASS()
class STATISTIC_API APlayerCharacter : public AMyCharacterBase, public IAnimationUpdateInterface, public IMyPlayerVisualInterface, public IAnimationWeaponInterface, public IImpactPointInterface
{
	GENERATED_BODY()

public:
	APlayerCharacter();

protected:
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
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
	void LeftClick();
	void ClickRelease();
	void RightClick();
	void BasicAttack();
	void SkillAttack();
	void QSkill();
	void ESkill();
	void Interaction();

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
	TObjectPtr<class UInputAction> IA_RightAttack;

	UPROPERTY(VisibleAnywhere, Category = Input, Meta = (AllowPrivateAccess = "true"))
	TObjectPtr<class UInputAction> IA_QSkill;

	UPROPERTY(VisibleAnywhere, Category = Input, Meta = (AllowPrivateAccess = "true"))
	TObjectPtr<class UInputAction> IA_ESkill;

	UPROPERTY(VisibleAnywhere, Category = Input, Meta = (AllowPrivateAccess = "true"))
	TObjectPtr<class UInputAction> IA_ChangeClass;

	UPROPERTY(VisibleAnywhere, Category = Input, Meta = (AllowPrivateAccess = "true"))
	TObjectPtr<class UInputAction> IA_Interaction;


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
	UPROPERTY(EditAnywhere, Category = Montage, Meta = (AllowPrivateAccess = "true"))
	TObjectPtr<class UAnimMontage> IceComboMontage;

// Combo
protected:
	UPROPERTY(EditAnywhere, Category = ComboData, Meta = (AllowPrivateAccess = "true"))
	TObjectPtr<class UMMComboActionData> StoneComboData;
	UPROPERTY(EditAnywhere, Category = ComboData, Meta = (AllowPrivateAccess = "true"))
	TObjectPtr<class UMMComboActionData> WindComboData;
	UPROPERTY(EditAnywhere, Category = ComboData, Meta = (AllowPrivateAccess = "true"))
	TObjectPtr<class UMMComboActionData> FireComboData;
	UPROPERTY(EditAnywhere, Category = ComboData, Meta = (AllowPrivateAccess = "true"))
	TObjectPtr<class UMMComboActionData> IceComboData;

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
	EClassType LeftClassType;
	EClassType RightClassType;
	
	// 클래스 변경 함수
	void ChangeClass(EClassType NewClassType, bool bIsLeft);

private:
    // 왼쪽인지 오른쪽인지 저장할 변수
    bool bIsLeft;

public:
    // 블루프린트에서 접근 가능한 변수
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Custom")
    uint8 CheckAnimBone : 1;

private:
	UAnimMontage* CurrentLeftMontage;
	UMMComboActionData* CurrentLeftComboData;
	FString CurrentLeftMontageSectionName; // 섹션 이름을 저장하는 변수

	UAnimMontage* CurrentRightMontage;
	UMMComboActionData* CurrentRightComboData;
	FString CurrentRightMontageSectionName; // 섹션 이름을 저장하는 변수

	// 캐싱된 데이터를 업데이트하는 함수 
	void UpdateCachedData(bool bIsLeftType);

	// 캐싱된 현재 몽타주와 데이터
	UAnimMontage* CurrentMontage;
	UMMComboActionData* CurrentComboData;
	FString CurrentMontageSectionName; // 섹션 이름을 저장하는 변수

	// MMPlayerCharacter Header
	void EquipWeapon(class AMyWeapon* Weapon, bool bIsLeftType);

	// TEST
	UPROPERTY(EditAnywhere, Category = Weapon, meta = (AllowPrivateAccess = "true"))
	TSubclassOf<class AMyWeapon> WeaponClass;

	UPROPERTY(VisibleAnywhere, Category = Weapon, meta = (AllowPrivateAccess = "true"))
	TObjectPtr<class AMyWeapon> CurrentLeftWeapon;

	UPROPERTY(VisibleAnywhere, Category = Weapon, meta = (AllowPrivateAccess = "true"))
	TObjectPtr<class AMyWeapon> CurrentRightWeapon;

	UPROPERTY(VisibleAnywhere, Category = Weapon, meta = (AllowPrivateAccess = "true"))
	TObjectPtr<class AMyWeapon> CurrentWeapon;

protected:
	FORCEINLINE virtual EClassType GetClassType() override { return RightClassType; };

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon")
    TSubclassOf<AMyWeapon> FireWeaponBP;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon")
    TSubclassOf<AMyWeapon> WindWeaponBP;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon")
    TSubclassOf<AMyWeapon> StoneWeaponBP;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon")
    TSubclassOf<AMyWeapon> IceWeaponBP;
private:
    bool bIsQDrawing = false; 
	bool bisEDrawing = false; 
    FVector CurrentImpactPoint;   // 현재 충돌 지점
	FRotator CurrentImpactRot;     // 현재 충돌 회전
    FTimerHandle CircleUpdateTimerHandle; // 원 업데이트를 위한 타이머 핸들
	FTimerHandle RectangleUpdateTimerHandle; // 사각형 업데이트를 위한 타이머 핸들
	void UpdateCircle();
	void UpdateRectangle();

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
	FTimerHandle SkillCastDelayTimerHandle;

	char m_id;
	float m_yaw;
	FVector m_velocity;
	char m_hp;
	char m_current_element[2];

	unsigned short m_skill_id;

	bool m_is_player;

	bool m_was_moving;

	FVector m_stop_location;
	bool m_is_stopping;

public:
	void do_send(void* buff);

	char get_id() { return m_id; }
	float get_yaw() { return GetControlRotation().Yaw; }
	FVector get_velocity() { return m_velocity; }
	char get_hp() { return m_hp; }
	char get_current_element(bool is_left) { return is_left ? m_current_element[0] : m_current_element[1]; }
	unsigned short get_skill_id() { return m_skill_id; }
	bool get_is_player() { return m_is_player; }

	void set_id(char id) { m_id = id; }
	void set_yaw(float yaw) { m_yaw = yaw; }
	void set_velocity(float x, float y, float z) { m_velocity.X = x; m_velocity.Y = y; m_velocity.Z = z; }
	void set_hp(char hp) { m_hp = hp; }
	void set_current_element(char current_element, bool is_left) { 
		if (is_left) m_current_element[0] = current_element; 
		else m_current_element[1] = current_element; 
	}
	void set_is_player(bool is_player) { m_is_player = is_player; }
	void set_stop_location(FVector stop_location) { m_stop_location = stop_location; }
	void set_is_stopping(bool is_stopping) { m_is_stopping = is_stopping; }

	void ready_skill(bool is_left);
	void use_skill(unsigned short skill_id, char skill_type, FVector v, bool is_left, float time);
	void use_skill(unsigned short skill_id, char skill_type, FVector v, FRotator r, bool is_left, float time);

	UFUNCTION()
	void InternalUseSkill_Vector(uint16 skill_id, uint8  skill_type, FVector v, bool is_left);
	UFUNCTION()
	void InternalUseSkill_Rotator(uint16 skill_id, uint8  skill_type, FVector v, FRotator r, bool is_left);

	void change_element();
	void change_element(char element_type, bool is_left);
	void rotate(float yaw);

	void Overlap(char skill_type, bool collision_start);

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite ,Category = "Status")
	float playerMaxHp = 100.0f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite ,Category = "Status")
	float playerCurrentHp = 100.0f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite ,Category = "Status")
	float playerMaxMp = 100.0f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite ,Category = "Status")
	float playerCurrentMp = 100.0f;

public:
    // PlayerWidget 선언
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "UI")
    TObjectPtr<class UPlayerWidget> CharacterWidget;

	// PlayerCharacter.h
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "UI")
	TSubclassOf<UUserWidget> PlayerWidgetClass;
private:
	// PlayerWidget 생성
	

	bool bCanUseSkillQ = true;
	float SkillQCoolTime = 5.0f;
	float CurrnetSkillQTime = 0.0f;

	bool bCanUseSkillE = true;
	float SkillECoolTime = 3.0f;
	float CurrnetSkillETime = 0.0f;

	bool bIsHold = false;
	bool bIsIceAiming = false;  // 얼음 조준 중 여부
	float DefaultArmLength = 300.0f;
	FVector DefaultCameraRelativeLocation = FVector(0.0f, 0.0f, 60.0f);

protected:
    // 발자국 효과음 배열
    UPROPERTY(EditAnywhere, Category = "Sound")
    TArray<TObjectPtr<class USoundBase>> FootstepSounds;

    // 발자국 효과음을 순서대로 재생하기 위한 인덱스
    int32 CurrentFootstepIndex = 0;

public:
	void UpdateUI();
	//UI숨기기
	void HideUI();
	//UI보이기
	void ShowUI();
    // 발자국 효과음을 재생하는 함수
    void PlayFootstepSound();
	void StartIceAim();
	void ShootIceArrow();

	//미니맵
public:
	UPROPERTY(VisibleAnywhere, Category = "MinimapCamera")
	class USpringArmComponent* minimapCameraBoom;

	UPROPERTY(VisibleAnywhere, Category = "MinimapCamera")
	class USceneCaptureComponent2D* minimapCapture;

	UPROPERTY(VisibleAnywhere, Category = "MinimapCamera")
	class UPaperSpriteComponent* minimapSprite;

public:
    bool bRecentlyTeleported = false;

	UPROPERTY()
	AActor* CurrentInteractTarget = nullptr;
	bool bIsInteraction = false; // 상호작용 여부
	bool bIsInteractionEnd = false; // 상호작용 종료 여부
	bool bIsInteractionWidgetOpen = false; // 상호작용 위젯 열림 여부
	bool bIsAttacking = false;

};