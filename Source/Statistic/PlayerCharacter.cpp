// Fill out your copyright notice in the Description page of Project Settings.

#include "PlayerCharacter.h"
#include "Components/CapsuleComponent.h"
#include "Camera/CameraComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "GameFramework/CharacterMovementComponent.h"

#include "PlayerWidget.h"
#include "InputAction.h"
#include "InputMappingContext.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "MMComboActionData.h" // Include the header for UMMComboActionData
#include "MyWeapon.h"
#include "MyFireWeapon.h"
#include "MyWindWeapon.h"
#include "MyStoneWeapon.h"
#include "MyIceWeapon.h"
#include "Kismet/GameplayStatics.h"

APlayerCharacter::APlayerCharacter()
{
	// Initialize
	m_id = -1;
	m_yaw = 0.0f;
	m_velocity = FVector(0.0f, 0.0f, 0.0f);
	m_hp = 100;
	m_current_element[0] = ELEMENT_WIND;
	m_current_element[1] = ELEMENT_FIRE;

	m_skill_id = 0;

	m_is_player = false;
	m_was_moving = false;

	// Collision 설정
	{
		GetCapsuleComponent()->InitCapsuleSize(35.0f, 90.0f);
	}

	// Mesh 설정
	{
		// Load
		static ConstructorHelpers::FObjectFinder<USkeletalMesh> SkeletalMeshRef(TEXT("/Game/ElfArden/BaseMesh/SK_ElfArden.SK_ElfArden"));
		if (SkeletalMeshRef.Succeeded())
		{
			GetMesh()->SetSkeletalMesh(SkeletalMeshRef.Object);

			// 메시 위치/회전 설정 (보통 캡슐 컴포넌트 기준 조정)
			GetMesh()->SetRelativeLocation(FVector(0.0f, 0.0f, -90.0f));
			GetMesh()->SetRelativeRotation(FRotator(0.0f, -90.0f, 0.0f));
		}	
		UE_LOG(LogTemp, Warning, TEXT("SkeletalMesh Load: %s"), SkeletalMeshRef.Succeeded() ? TEXT("Success") : TEXT("Fail"));
	}

	// Camera 설정
	{
		SpringArm = CreateDefaultSubobject<USpringArmComponent>(TEXT("SpringArm"));
		SpringArm->SetupAttachment(RootComponent);

		Camera = CreateDefaultSubobject<UCameraComponent>(TEXT("Camera"));
		Camera->SetupAttachment(SpringArm, USpringArmComponent::SocketName);
	}

	// Input
    {
        static ConstructorHelpers::FObjectFinder<UInputMappingContext> IMC_BasicRef(TEXT("/Script/EnhancedInput.InputMappingContext'/Game/input/IMC_BasicPlayer.IMC_BasicPlayer'"));
        if (IMC_BasicRef.Object)
        {
            IMC_Basic = IMC_BasicRef.Object;
        }

        static ConstructorHelpers::FObjectFinder<UInputAction> IA_BasicMoveRef(TEXT("/Script/EnhancedInput.InputAction'/Game/input/IA_BaseMove.IA_BaseMove'"));
        if (IA_BasicMoveRef.Object)
        {
            IA_BasicMove = IA_BasicMoveRef.Object;
        }

        static ConstructorHelpers::FObjectFinder<UInputAction> IA_BasicLookRef(TEXT("/Script/EnhancedInput.InputAction'/Game/input/IA_BaseLook.IA_BaseLook'"));
        if (IA_BasicLookRef.Object)
        {
            IA_BasicLook = IA_BasicLookRef.Object;
        }

        static ConstructorHelpers::FObjectFinder<UInputAction> IA_BasicJumpRef(TEXT("/Script/EnhancedInput.InputAction'/Game/input/IA_BaseJump.IA_BaseJump'"));
        if (IA_BasicJumpRef.Object)
        {
            IA_BasicJump = IA_BasicJumpRef.Object;
        }
		static ConstructorHelpers::FObjectFinder<UInputAction>IA_DashRef(TEXT("/Script/EnhancedInput.InputAction'/Game/input/IA_Dash.IA_Dash'"));
		if (IA_DashRef.Object)
		{
			IA_Dash = IA_DashRef.Object;
		}
		static ConstructorHelpers::FObjectFinder<UInputAction>IA_BasicAttackRef(TEXT("/Script/EnhancedInput.InputAction'/Game/input/attack/IA_BaseAttack.IA_BaseAttack'"));
		if (IA_BasicAttackRef.Object)
		{
			IA_BasicAttack = IA_BasicAttackRef.Object;
		}
		static ConstructorHelpers::FObjectFinder<UInputAction>IA_RightAttackRef(TEXT("/Script/EnhancedInput.InputAction'/Game/input/attack/IA_RightAttack.IA_RightAttack'"));
		if (IA_RightAttackRef.Object)
		{
			IA_RightAttack = IA_RightAttackRef.Object;
		}
		static ConstructorHelpers::FObjectFinder<UInputAction>IA_QSkillRef(TEXT("/Script/EnhancedInput.InputAction'/Game/input/IA_QSkill.IA_QSkill'"));
		if (IA_QSkillRef.Object)
		{
			IA_QSkill = IA_QSkillRef.Object;
		}
		static ConstructorHelpers::FObjectFinder<UInputAction>IA_RSkillRef(TEXT("/Script/EnhancedInput.InputAction'/Game/input/IA_ESkill.IA_ESkill'"));
		if (IA_RSkillRef.Object)
		{
			IA_ESkill = IA_RSkillRef.Object;
		}

		static ConstructorHelpers::FObjectFinder<UInputAction>IA_ChangeClassRef(TEXT("/Script/EnhancedInput.InputAction'/Game/input/IA_ChangeClass.IA_ChangeClass'"));
		if (IA_ChangeClassRef.Object)
		{
			IA_ChangeClass = IA_ChangeClassRef.Object;
		}
	}

	// Setting (기본적으로 원하는 기본 이동을 위한 캐릭터 설정)
	{
		// 컨트롤러의 Rotation에 영향 X
		bUseControllerRotationPitch = false;
		bUseControllerRotationYaw = true;
		bUseControllerRotationRoll = false;

		// 폰의 컨트롤 회전 사용
		SpringArm->bUsePawnControlRotation = true;
		// 움직임에 따른 회전 On
		GetCharacterMovement()->bOrientRotationToMovement = false;
		// 점프 높이 설정
        GetCharacterMovement()->JumpZVelocity = 400.0f; // 원하는 값으로 설정
		GetCharacterMovement()->MaxWalkSpeed = 280.0f;
		 // Member Variable 초기화
	}

	{
		bIsDash = false;
	}

    LeftClassType = EClassType::CT_None;
	RightClassType = EClassType::CT_None;

    // 초기 캐싱된 데이터 설정
    CurrentLeftMontage = nullptr;
    CurrentLeftComboData = nullptr;

	CurrentRightMontage = nullptr;
    CurrentRightComboData = nullptr;

	static ConstructorHelpers::FClassFinder<AMyWeapon> FireWeaponBPRef(TEXT("/Game/Weapon/BP_FIreWeapon.BP_FIreWeapon_C"));
    if (FireWeaponBPRef.Succeeded())
    {
        FireWeaponBP = FireWeaponBPRef.Class;
    }
	static ConstructorHelpers::FClassFinder<AMyWeapon> WindWeaponBPRef(TEXT("/Game/Weapon/BP_WindWeapon.BP_WindWeapon_C"));
    if (WindWeaponBPRef.Succeeded())
    {
        WindWeaponBP = WindWeaponBPRef.Class;
    }
    // StoneWeaponBP 초기화
    static ConstructorHelpers::FClassFinder<AMyWeapon> StoneWeaponBPRef(TEXT("/Game/Weapon/BP_StoneWeapon.BP_StoneWeapon_C"));
    if (StoneWeaponBPRef.Succeeded())
    {
        StoneWeaponBP = StoneWeaponBPRef.Class;
    }
	static ConstructorHelpers::FClassFinder<AMyWeapon> IceWeaponBPRef(TEXT("/Game/Weapon/BP_IceWeapon.BP_IceWeapon_C"));
    if (IceWeaponBPRef.Succeeded())
    {
        IceWeaponBP = IceWeaponBPRef.Class;
    }

}

// BeginPlay 안의 기존 부분을 교체
void APlayerCharacter::BeginPlay()
{
    Super::BeginPlay();

	SetActorLocation(FVector(0.0f, 0.0f, 0.0f));

    APlayerController* PlayerController = Cast<APlayerController>(GetController());
    if (PlayerController && IMC_Basic)
    {
        if (UEnhancedInputLocalPlayerSubsystem* SubSystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(PlayerController->GetLocalPlayer()))
        {
            SubSystem->AddMappingContext(IMC_Basic, 0);
            EnableInput(PlayerController);
        } 
    }

    ChangeClass(EClassType::CT_Ice, true);
    ChangeClass(EClassType::CT_Stone, false);

    playerCurrentHp = playerMaxHp;
    playerCurrentMp = playerMaxMp;

	if (PlayerController && PlayerWidgetClass)
	{
		CharacterWidget = CreateWidget<UPlayerWidget>(PlayerController, PlayerWidgetClass);
		if (CharacterWidget)
		{
			CharacterWidget->AddToViewport();
			UpdateUI();
		}
	}
	DefaultArmLength = SpringArm->TargetArmLength;
	DefaultCameraRelativeLocation = Camera->GetRelativeLocation();

}

void APlayerCharacter::EndPlay(const EEndPlayReason::Type EndPlayReason) {
	Super::EndPlay(EndPlayReason);

	if (CurrentLeftWeapon) { 
		CurrentLeftWeapon->Destroy(); 
	}

	if (CurrentRightWeapon) {
		CurrentRightWeapon->Destroy();
	}
}

void APlayerCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	// 향상된 입력 컴포넌트로 형변환
    // * CastChecked : 캐스팅이 실패할 경우 프로그램을 중단시키는 대신 에러 메시지 출력
	UEnhancedInputComponent* EnhancedInputComponent = CastChecked<UEnhancedInputComponent>(PlayerInputComponent);

	// 생성된 IA_BasicLook, IA_BasicMove와 함수 매핑
	EnhancedInputComponent->BindAction(IA_BasicLook, ETriggerEvent::Triggered, this, &APlayerCharacter::BasicLook);
	EnhancedInputComponent->BindAction(IA_BasicMove, ETriggerEvent::Triggered, this, &APlayerCharacter::BasicMove);
	
    EnhancedInputComponent->BindAction(IA_BasicJump, ETriggerEvent::Triggered, this, &APlayerCharacter::StartJump);
    EnhancedInputComponent->BindAction(IA_BasicJump, ETriggerEvent::Completed, this, &APlayerCharacter::StopJump);
	
	EnhancedInputComponent->BindAction(IA_Dash, ETriggerEvent::Triggered, this, &APlayerCharacter::DashStart);
	EnhancedInputComponent->BindAction(IA_Dash, ETriggerEvent::Completed, this, &APlayerCharacter::DashEnd);

	EnhancedInputComponent->BindAction(IA_BasicAttack, ETriggerEvent::Started, this, &APlayerCharacter::LeftClick);
	EnhancedInputComponent->BindAction(IA_BasicAttack, ETriggerEvent::Completed, this, &APlayerCharacter::ClickRelease);

	EnhancedInputComponent->BindAction(IA_RightAttack, ETriggerEvent::Started, this, &APlayerCharacter::RightClick);
	EnhancedInputComponent->BindAction(IA_RightAttack, ETriggerEvent::Completed, this, &APlayerCharacter::ClickRelease);

	EnhancedInputComponent->BindAction(IA_QSkill, ETriggerEvent::Triggered, this, &APlayerCharacter::QSkill);
	EnhancedInputComponent->BindAction(IA_ESkill, ETriggerEvent::Triggered, this, &APlayerCharacter::ESkill);

	EnhancedInputComponent->BindAction(IA_ChangeClass, ETriggerEvent::Triggered, this, &APlayerCharacter::change_element);
}

void APlayerCharacter::BasicMove(const FInputActionValue& Value)
{
	if (!(GetCharacterMovement()->IsFalling())) {
		// 입력받은 Value로부터 MovementVector 가져오기
		MovementVector = Value.Get<FVector2D>();

		// 컨트롤러의 회전 중 Yaw(Z)를 가져와 저장
		const FRotator Rotation = Controller->GetControlRotation();
		const FRotator YawRotation(0, Rotation.Yaw, 0);

		// 회전(Yaw)을 기반으로 전방 및 오른쪽 방향을 받아오기 (X : 전방, Y : 오른쪽)
		const FVector ForwardDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);
		const FVector RightDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);

		// 뒤로 이동할 때 최대 속도를 300으로 제한
		if (MovementVector.X < 0)
		{
			CheckBackMove = true;
			GetCharacterMovement()->MaxWalkSpeed = 220.0f;
		}
		else
		{
			CheckBackMove = false;

			// 대시 상태에 따라 최대 속도 설정
			if (bIsDash)
			{
				GetCharacterMovement()->MaxWalkSpeed = 600.0f;
			}
			else
			{
				GetCharacterMovement()->MaxWalkSpeed = 280.0f;
			}
		}

		// Movement에 값 전달 (방향, 이동량)
		AddMovementInput(ForwardDirection, MovementVector.X);
		AddMovementInput(RightDirection, MovementVector.Y);
	
		// Send Player Vector Packet 
		FVector Velocity = GetCharacterMovement()->Velocity;

		float DistanceDiff = FVector::Dist(Velocity, m_velocity);

		if (DistanceDiff > 10.0f) {
			m_velocity = Velocity;
			m_was_moving = true;

			FVector Position = GetActorLocation();

			player_vector_packet p;
			p.packet_size = sizeof(player_vector_packet);
			p.packet_type = C2H_PLAYER_VECTOR_PACKET;
			p.player_id = m_id;
			p.player_x = Position.X; p.player_y = Position.Y; p.player_z = Position.Z;
			p.player_vx = Velocity.X; p.player_vy = Velocity.Y; p.player_vz = Velocity.Z;

			do_send(&p);
			//UE_LOG(LogTemp, Warning, TEXT("[Client %d] Send Vector Packet to Host"), m_id);
		}
	}
}

void APlayerCharacter::BasicLook(const FInputActionValue& Value)
{
    // 입력받은 Value로부터 LookVector 가져오기
    FVector2D LookVector = Value.Get<FVector2D>();
    
    // Controller에 값 전달
    AddControllerYawInput(LookVector.X);
    AddControllerPitchInput(LookVector.Y);

    // 카메라 회전에 따라 캐릭터가 회전하도록 설정
    bUseControllerRotationYaw = true;
    GetCharacterMovement()->bOrientRotationToMovement = false;

	// Send Player Direction Packet 
	float CurrentYaw = GetControlRotation().Yaw;
	float YawDiff = FMath::Abs(CurrentYaw - m_yaw);

	if (YawDiff > 30.0f) { 
		m_yaw = CurrentYaw;

		player_rotate_packet p;
		p.packet_size = sizeof(player_rotate_packet);
		p.packet_type = C2H_PLAYER_ROTATE_PACKET;
		p.player_id = m_id;
		p.player_yaw = CurrentYaw;

		do_send(&p);
		//UE_LOG(LogTemp, Warning, TEXT("[Client %d] Send Rotation Packet to Host"), m_id);
	}
}

void APlayerCharacter::StartJump()
{
    bPressedJump = true;

	if (!(GetCharacterMovement()->IsFalling())) {
		player_jump_packet p;
		p.packet_size = sizeof(player_jump_packet);
		p.packet_type = C2H_PLAYER_JUMP_PACKET;
		p.player_id = m_id;

		do_send(&p);
		//UE_LOG(LogTemp, Warning, TEXT("[Client %d] Send Jump Start Packet to Host"), m_id);
	}
}

void APlayerCharacter::StopJump()
{
    bPressedJump = false;
}

void APlayerCharacter::DashStart()
{
	if(!CheckBackMove)
	{
		bIsDash = true;
	}
}

void APlayerCharacter::DashEnd()
{
	bIsDash = false;
}	

void APlayerCharacter::LeftClick()
{
    bIsLeft = true;

    // 왼쪽 무기가 얼음 타입인지 확인
    if (LeftClassType == EClassType::CT_Ice)
    {
		if(bIsQDrawing)
		{
			SkillAttack();
			return;
		}
		player_ice_aim_packet p;
		p.packet_size = sizeof(player_ice_aim_packet);
		p.packet_type = C2H_PLAYER_ICE_AIM_PACKET;
		p.player_id = m_id;
		do_send(&p);
		return;
    }

    BasicAttack();
}
void APlayerCharacter::RightClick()
{
	bIsLeft = false;

	if (RightClassType == EClassType::CT_Ice)
    {
		player_ice_aim_packet p;
		p.packet_size = sizeof(player_ice_aim_packet);
		p.packet_type = C2H_PLAYER_ICE_AIM_PACKET;
		p.player_id = m_id;
		do_send(&p);
		return;
    }

	BasicAttack();
}
void APlayerCharacter::StartIceAim()
{
	this->CurrentMontage = bIsLeft ? CurrentLeftMontage : CurrentRightMontage;
    this->CurrentComboData = bIsLeft ? CurrentLeftComboData : CurrentRightComboData;
    this->CurrentMontageSectionName = bIsLeft ? CurrentLeftMontageSectionName : CurrentRightMontageSectionName;
    this->CurrentWeapon = bIsLeft ? CurrentLeftWeapon : CurrentRightWeapon;

    bIsHold = true;
    bIsIceAiming = true;

    UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();
    if (AnimInstance && CurrentMontage)
    {
        if (!AnimInstance->Montage_IsPlaying(CurrentMontage))
        {
            AnimInstance->Montage_Play(CurrentMontage, 1.0f);
        }
        AnimInstance->Montage_JumpToSection(FName("IceAttack1"), CurrentMontage);
    }
}

void APlayerCharacter::ClickRelease()
{
	if(bIsLeft){
		if (LeftClassType == EClassType::CT_Ice) {
			GetFireTargetLocation();

			player_skill_vector_packet p;
			p.packet_size = sizeof(player_skill_vector_packet);
			p.packet_type = C2H_PLAYER_SKILL_VECTOR_PACKET;
			p.player_id = m_id;
			p.skill_vx = FireLocation.X; p.skill_vy = FireLocation.Y; p.skill_vz = FireLocation.Z;
			p.skill_type = SKILL_ICE_ARROW;
			p.is_left = true;
			do_send(&p);
		}
	}
	else{
		if (RightClassType == EClassType::CT_Ice) {
			GetFireTargetLocation();

			player_skill_vector_packet p;
			p.packet_size = sizeof(player_skill_vector_packet);
			p.packet_type = C2H_PLAYER_SKILL_VECTOR_PACKET;
			p.player_id = m_id;
			p.skill_vx = FireLocation.X; p.skill_vy = FireLocation.Y; p.skill_vz = FireLocation.Z;
			p.skill_type = SKILL_ICE_ARROW;
			p.is_left = false;
			do_send(&p);
		}
	}
}

void APlayerCharacter::ShootIceArrow() 
{
	bIsHold = false;
	bIsIceAiming = false;

	// 발사 처리
	UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();
	if (AnimInstance && CurrentMontage)
	{
		// 이미 재생 중이면 IceShoot 섹션으로 이동
		if (AnimInstance->Montage_IsPlaying(CurrentMontage))
		{
			AnimInstance->Montage_JumpToSection(FName("IceShoot"), CurrentMontage);
		}
		else
		{
			AnimInstance->Montage_Play(CurrentMontage, 1.0f);
			AnimInstance->Montage_JumpToSection(FName("IceShoot"), CurrentMontage);
		}
	}
}

//---------------------------------------------------------------------------------------------------------------------
void APlayerCharacter::BasicAttack()
{
	//UE_LOG(LogTemp, Error, TEXT("FireLocation: %s"), *FireLocation.ToString());
	//UE_LOG(LogTemp, Error, TEXT("CurrentImpactPoint: %s"), *CurrentImpactPoint.ToString());
	//UE_LOG(LogTemp, Error, TEXT("CurrentImpactRot: %s"), *CurrentImpactRot.ToString());

	EClassType ClassType = bIsLeft ? LeftClassType : RightClassType;

	if (bIsQDrawing)
	{
		if(!bIsLeft) return;
	}

	if (bisEDrawing)
	{
		if(bIsLeft) return;
	}

	if (bIsQDrawing || bisEDrawing)
    {
		UE_LOG(LogTemp, Error, TEXT("CurrentImpactPoint: %s"), *CurrentImpactPoint.ToString());
		UE_LOG(LogTemp, Error, TEXT("CurrentImpactRot: %s"), *CurrentImpactRot.ToString());

		// Send Skill Packet 
		if (get_is_player()) {
			switch (ClassType) {
			case EClassType::CT_Wind: {
				player_skill_vector_packet p;
				p.packet_size = sizeof(player_skill_vector_packet);
				p.packet_type = C2H_PLAYER_SKILL_VECTOR_PACKET;
				p.player_id = m_id;
				p.skill_vx = CurrentImpactPoint.X; p.skill_vy = CurrentImpactPoint.Y; p.skill_vz = CurrentImpactPoint.Z;
				p.skill_type = SKILL_WIND_TORNADO;
				p.is_left = bIsLeft;
				
				do_send(&p);
				break;
			}

			case EClassType::CT_Fire: {
				player_skill_rotator_packet p;
				p.packet_size = sizeof(player_skill_rotator_packet);
				p.packet_type = C2H_PLAYER_SKILL_ROTATOR_PACKET;
				p.player_id = m_id;
				p.skill_x = CurrentImpactPoint.X; p.skill_y = CurrentImpactPoint.Y; p.skill_z = CurrentImpactPoint.Z;
				p.skill_pitch = CurrentImpactRot.Pitch; p.skill_yaw = CurrentImpactRot.Yaw; p.skill_roll = CurrentImpactRot.Roll;
				p.skill_type = SKILL_FIRE_WALL;
				p.is_left = bIsLeft;

				do_send(&p);
				break;
			}

			case EClassType::CT_Stone: {
				player_skill_vector_packet p;
				p.packet_size = sizeof(player_skill_vector_packet);
				p.packet_type = C2H_PLAYER_SKILL_VECTOR_PACKET;
				p.player_id = m_id;
				p.skill_vx = CurrentImpactPoint.X; p.skill_vy = CurrentImpactPoint.Y; p.skill_vz = CurrentImpactPoint.Z;
				p.skill_type = SKILL_STONE_SKILL;
				p.is_left = bIsLeft;

				do_send(&p);
				break;
			}
			}
		}
        return;
    }

	if (CurrentComboCount == 0)
	{
		// Send Skill Packet
		if (get_is_player()) {
			GetFireTargetLocation();
			UE_LOG(LogTemp, Error, TEXT("FireLocation: %s"), *FireLocation.ToString());

			player_skill_vector_packet p;
			p.packet_size = sizeof(player_skill_vector_packet);
			p.packet_type = C2H_PLAYER_SKILL_VECTOR_PACKET;
			p.player_id = m_id;
			p.skill_vx = FireLocation.X; p.skill_vy = FireLocation.Y; p.skill_vz = FireLocation.Z;
			p.is_left = bIsLeft;

			switch (ClassType) {
			case EClassType::CT_Wind:
				p.skill_type = SKILL_WIND_CUTTER;
				break;

			case EClassType::CT_Fire:
				p.skill_type = SKILL_FIRE_BALL;
				break;

			case EClassType::CT_Stone:
				p.skill_type = SKILL_STONE_WAVE;
				break;
			}

			do_send(&p);
		}
		return;
	}
	
	// * 콤보 타이머가 종료되지 않은 상태라면 콤보 입력 체크
	if (ComboTimerHandle.IsValid())
	{
		bHasComboInput = true;
	}
	// * 콤보 타이머가 유효하지 않은(종료) 상태라면 콤보 입력 체크 해제
	else
	{
		bHasComboInput = false;
	}
}

void APlayerCharacter::SkillAttack()
{
	if (bIsLeft)
    {
        if (!bCanUseSkillQ) return;
    }
    else
    {
        if (!bCanUseSkillE) return;
    }

    this->CurrentMontage = bIsLeft ? CurrentLeftMontage : CurrentRightMontage;
    this->CurrentComboData = bIsLeft ? CurrentLeftComboData : CurrentRightComboData;
    this->CurrentMontageSectionName = bIsLeft ? CurrentLeftMontageSectionName : CurrentRightMontageSectionName;
    this->CurrentWeapon = bIsLeft ? CurrentLeftWeapon : CurrentRightWeapon;

    if (playerCurrentMp >= 20)
    {
        playerCurrentMp -= 20.0f;

        if (bIsLeft)
        {
            bCanUseSkillQ = false;
            CurrnetSkillQTime = 0.0f;

            if (CharacterWidget)
                CharacterWidget->UpdateCountDown(SkillQCoolTime, bIsLeft);
        }
        else
        {
            bCanUseSkillE = false;
            CurrnetSkillETime = 0.0f;

            if (CharacterWidget)
                CharacterWidget->UpdateCountDown(SkillECoolTime, bIsLeft);
        }

        UpdateUI();
    }

    UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();

    UE_LOG(LogTemp, Warning, TEXT("AnimInstance: %s"), AnimInstance ? TEXT("Valid") : TEXT("Invalid"));

    if (AnimInstance && CurrentMontage)
    {
        // 현재 몽타주가 재생 중인지 확인
        if (AnimInstance->Montage_IsPlaying(CurrentMontage))
        {
            return; // 몽타주가 재생 중이면 함수 종료
        }

        // 섹션 이름 설정
        FName SectionName = FName(*CurrentMontageSectionName);

		float AttackSpeedRate = 4.0f;
		if (CurrentWeapon && CurrentWeapon->IsA(AMyIceWeapon::StaticClass()))
		{
			AttackSpeedRate = 1.0f;
		}
        // 몽타주 재생
        AnimInstance->Montage_Play(CurrentMontage, AttackSpeedRate);
        AnimInstance->Montage_JumpToSection(SectionName, CurrentMontage);

        // 다음 콤보를 위한 입력 초기화 및 타이머 재설정
        SetComboTimer();
        bHasComboInput = false;
    }
}


void APlayerCharacter::ComboStart()
{
	this->CurrentMontage = bIsLeft ? CurrentLeftMontage : CurrentRightMontage;
	this->CurrentComboData = bIsLeft ? CurrentLeftComboData : CurrentRightComboData;
	this->CurrentMontageSectionName = bIsLeft ? CurrentLeftMontageSectionName : CurrentRightMontageSectionName;
	this->CurrentWeapon = bIsLeft ? CurrentLeftWeapon : CurrentRightWeapon;

    CurrentComboCount = 1;
    float AttackSpeedRate = 4.0f;
    if (CurrentWeapon && CurrentWeapon->IsA(AMyStoneWeapon::StaticClass()))
    {
        AttackSpeedRate = 2.0f;
    }
	
    UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();
	UE_LOG(LogTemp, Warning, TEXT("AnimInstance: %s"), AnimInstance ? TEXT("Valid") : TEXT("Invalid"));
	UE_LOG(LogTemp, Warning, TEXT("CurrentMontage: %s"), CurrentMontage ? TEXT("Valid") : TEXT("Invalid"));

    // 몽타주 재생
    float MontageLength = AnimInstance->Montage_Play(CurrentMontage, AttackSpeedRate);

    if (MontageLength > 0.0f)
    {
        UE_LOG(LogTemp, Warning, TEXT("Montage_Play succeeded! Montage Length: %f"), MontageLength);
		
        // 몽타주 재생 종료 바인딩
        FOnMontageEnded EndDelegate;
        EndDelegate.BindUObject(this, &APlayerCharacter::ComboEnd);

        // BasicComboMontage가 종료되면 EndDelegate에 연동된 ComboEnd 함수 호출
        AnimInstance->Montage_SetEndDelegate(EndDelegate, CurrentMontage);

        // 타이머 초기화
        ComboTimerHandle.Invalidate();

        // 타이머 설정
        SetComboTimer(); 
    }
}

void APlayerCharacter::ComboEnd(UAnimMontage* Montage, bool IsEnded)
{
	UE_LOG(LogTemp, Warning, TEXT("ComboEnd"));

	// 콤보 수 초기화
	CurrentComboCount = 0;

	// 콤보 입력 판별 초기화
	bHasComboInput = false;
}

void APlayerCharacter::ComboCheck()
{
	ComboTimerHandle.Invalidate();

	if (bHasComboInput)
	{
		CurrentComboCount = FMath::Clamp(CurrentComboCount + 1, 1, CurrentComboData->MaxComboCount);

		UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();
		if (AnimInstance && CurrentMontage)
		{
			// 현재 섹션 이름 얻기
			FName CurrentSectionName = AnimInstance->Montage_GetCurrentSection(CurrentMontage);

			// 다음 섹션 이름 생성
			FName NextSectionName = *FString::Printf(TEXT("%s%d"), *CurrentComboData->SectionPrefix, CurrentComboCount);

			// 현재 섹션 끝나면 다음 섹션으로 자연스럽게 블렌드되도록 설정
			AnimInstance->Montage_SetNextSection(CurrentSectionName, NextSectionName, CurrentMontage);

			// 다음 콤보를 위한 입력 초기화 및 타이머 재설정
			SetComboTimer();
			bHasComboInput = false;
		}
	}
}

void APlayerCharacter::ChangeClass(EClassType NewClassType, bool bIsLeftType)
{
	EClassType& TargetClassType = bIsLeftType ? LeftClassType : RightClassType;

    if (TargetClassType != NewClassType)
    {
        TargetClassType = NewClassType;
        UpdateCachedData(bIsLeftType);
        UE_LOG(LogTemp, Warning, TEXT("%s Class Type Changed to: %d"), bIsLeftType ? TEXT("Left") : TEXT("Right"), static_cast<int32>(TargetClassType));
    }
}

void APlayerCharacter::UpdateCachedData(bool bIsLeftType)
{
    if (bIsLeftType)
    {
        if (CurrentLeftWeapon)
        {
            CurrentLeftWeapon->Destroy();
            CurrentLeftWeapon = nullptr;
        }
    }
    else
    {
        if (CurrentRightWeapon)
        {
            CurrentRightWeapon->Destroy();
            CurrentRightWeapon = nullptr;
        }
    }

    // 클래스 타입에 따라 무기와 데이터를 업데이트
    EClassType ClassTypeToUpdate = bIsLeftType ? LeftClassType : RightClassType;
    UAnimMontage*& SelectedMontage = bIsLeftType ? CurrentLeftMontage : CurrentRightMontage;
    UMMComboActionData*& SelectedComboData = bIsLeftType ? CurrentLeftComboData : CurrentRightComboData;
    FString& SelectedMontageSectionName = bIsLeftType ? CurrentLeftMontageSectionName : CurrentRightMontageSectionName;
	
    switch (ClassTypeToUpdate)
    {
    case EClassType::CT_Wind:
        SelectedMontage = WindComboMontage;
        SelectedComboData = WindComboData;
        WeaponClass = WindWeaponBP;
        SelectedMontageSectionName = TEXT("WindSkill");
        CheckAnimBone = 1;
		break;

    case EClassType::CT_Stone:
        SelectedMontage = StoneComboMontage;
        SelectedComboData = StoneComboData;
		WeaponClass = StoneWeaponBP;
		SelectedMontageSectionName = TEXT("StoneSkill");
        CheckAnimBone = 0;
        break;

    case EClassType::CT_Fire:
        SelectedMontage = FireComboMontage;
        SelectedComboData = FireComboData;
        WeaponClass = FireWeaponBP;
        SelectedMontageSectionName = TEXT("FireSkill");
        CheckAnimBone = 1;
		break;
    case EClassType::CT_Ice:
        SelectedMontage = IceComboMontage;
        SelectedComboData = IceComboData;
        WeaponClass = IceWeaponBP;
        SelectedMontageSectionName = TEXT("IceSkill");
        CheckAnimBone = 1;
		break;
    default:
        SelectedMontage = nullptr;
        SelectedComboData = nullptr;
        break;

    }

    if (GetWorld() && WeaponClass)
    {
        if (bIsLeftType)
        {
            CurrentLeftWeapon = Cast<AMyWeapon>(GetWorld()->SpawnActor<AMyWeapon>(WeaponClass));

            if (CurrentLeftWeapon)
            {
                EquipWeapon(CurrentLeftWeapon, bIsLeftType);
            }
        }
        else
        {
            CurrentRightWeapon = Cast<AMyWeapon>(GetWorld()->SpawnActor<AMyWeapon>(WeaponClass));
            if (CurrentRightWeapon)
            {
                EquipWeapon(CurrentRightWeapon, bIsLeftType);
            }
        }
    }
	
	if (bIsLeftType)
	{
		CharacterWidget->SetQSkillIcon(LeftClassType);
	}
	else
	{
		CharacterWidget->SetESkillIcon(RightClassType);
	}
}

void APlayerCharacter::SetComboTimer()
{
    if (!CurrentComboData)
    {
        UE_LOG(LogTemp, Error, TEXT("ComboData is missing for the current class type!"));
        return;
    }

    int32 ComboIndex = CurrentComboCount - 1;

    // 인덱스가 유효한지 체크
    if (CurrentComboData->ComboFrame.IsValidIndex(ComboIndex))
    {
        const float AttackSpeedRate = 2.0f;

        // 실제 콤보가 입력될 수 있는 시간 구하기
        float ComboAvailableTime = (CurrentComboData->ComboFrame[ComboIndex] / CurrentComboData->FrameRate) / AttackSpeedRate;

        // 타이머 설정하기
        if (ComboAvailableTime > 0.0f)
        {
            GetWorld()->GetTimerManager().SetTimer(ComboTimerHandle, this, &APlayerCharacter::ComboCheck, ComboAvailableTime, false);
        }
    }
}

void APlayerCharacter::EquipWeapon(AMyWeapon* Weapon, bool bIsLeftType)
{
	Weapon->EquipWeapon(this, bIsLeftType);
}

void APlayerCharacter::Tick(float DeltaTime) {
	Super::Tick(DeltaTime);

	float MpRenRate = 10.0f;
	playerCurrentMp = FMath::Clamp(playerCurrentMp + MpRenRate * DeltaTime, 0.0f, playerMaxMp);

	if(!bCanUseSkillQ)
	{
		CurrnetSkillQTime += DeltaTime;
		if(CurrnetSkillQTime >= SkillQCoolTime)
		{
			bCanUseSkillQ = true;
			CurrnetSkillQTime = 0.0f;
		}
	}
	if(!bCanUseSkillE)
	{
		CurrnetSkillETime += DeltaTime;
		if(CurrnetSkillETime >= SkillECoolTime)
		{
			bCanUseSkillE = true;
			CurrnetSkillETime = 0.0f;
		}
	}
	UpdateUI();
	if (m_is_player) {
		// Stop
		if (m_was_moving) {
			FVector Velocity = GetCharacterMovement()->Velocity;
			//UE_LOG(LogTemp, Warning, TEXT("[Client %d] VX : %.2f, VY : %.2f"), m_id, Velocity.X, Velocity.Y);

			if (Velocity.IsNearlyZero()) {
				m_velocity = FVector(0.0f, 0.0f, 0.0f);
				m_was_moving = false;

				FVector Position = GetActorLocation();

				player_stop_packet p;
				p.packet_size = sizeof(player_stop_packet);
				p.packet_type = C2H_PLAYER_STOP_PACKET;
				p.player_id = m_id;
				p.player_x = Position.X; p.player_y = Position.Y; p.player_z = Position.Z;

				do_send(&p);
				//UE_LOG(LogTemp, Warning, TEXT("[Client %d] Send Stop Packet to Host"), m_id);
			}
		}
	}
	else {
		if (!GetCharacterMovement()->IsFalling()) {
			GetCharacterMovement()->Velocity = m_velocity;

			float Speed = m_velocity.Size();

			// Dash
			if (Speed > 500.0f) {
				GetCharacterMovement()->MaxWalkSpeed = 600.0f;
			}
			// Walk
			else {
				GetCharacterMovement()->MaxWalkSpeed = 280.0f;
			}

			AddMovementInput(m_velocity.GetSafeNormal(), 1.0f);
		}
	}

	SleepEx(0, TRUE);

	float InterpSpeed = 2.0f;

	if (bIsHold)
	{
		// 확대: 암 길이를 줄임
		SpringArm->TargetArmLength = FMath::FInterpTo(SpringArm->TargetArmLength, 200.0f, DeltaTime, InterpSpeed);

		// 캐릭터 회전: 카메라 방향으로 부드럽게 회전
		FRotator StartRot = GetActorRotation();
		FRotator TargetRot = FRotator(StartRot.Pitch, GetControlRotation().Yaw, StartRot.Roll);
		SetActorRotation(FMath::RInterpTo(StartRot, TargetRot, DeltaTime, InterpSpeed));
	}
	else
	{
		// 복귀: 원래 거리로 돌아감
		SpringArm->TargetArmLength = FMath::FInterpTo(SpringArm->TargetArmLength, DefaultArmLength, DeltaTime, InterpSpeed);
	}

}

void APlayerCharacter::QSkill()
{
    if (bIsQDrawing)
    {
        // QSkill 취소
        bIsQDrawing = false;
        GetWorld()->GetTimerManager().ClearTimer(CircleUpdateTimerHandle);
        UE_LOG(LogTemp, Warning, TEXT("QSkill canceled."));
    }
    else
    {
        if (bisEDrawing)
        {
            // ESkill 취소
            bisEDrawing = false;
            GetWorld()->GetTimerManager().ClearTimer(RectangleUpdateTimerHandle);
            UE_LOG(LogTemp, Warning, TEXT("ESkill canceled for QSkill."));
        }

        // QSkill 활성화
        bIsQDrawing = true;

        // 카메라 위치와 방향 가져오기
        FVector CameraLocation;
        FRotator CameraRotation;
        GetController()->GetPlayerViewPoint(CameraLocation, CameraRotation);

        // 라인트레이스 시작점과 끝점 설정
        FVector Start = CameraLocation;
        FVector End = Start + (CameraRotation.Vector() * 6000.0f);

        // 충돌 파라미터 설정
        FHitResult HitResult;
        FCollisionQueryParams CollisionParams;
        CollisionParams.AddIgnoredActor(this);

        // 라인트레이스 실행
        bool bHit = GetWorld()->LineTraceSingleByChannel(HitResult, Start, End, ECC_Visibility, CollisionParams);

        if (bHit)
        {
            // 충돌 지점 저장
            CurrentImpactPoint = HitResult.ImpactPoint;

            // 원 업데이트 타이머 시작
            GetWorld()->GetTimerManager().SetTimer(CircleUpdateTimerHandle, this, &APlayerCharacter::UpdateCircle, 0.1f, true);
            UE_LOG(LogTemp, Warning, TEXT("QSkill activated."));
        }
    }
}

void APlayerCharacter::ESkill()
{
    if (bisEDrawing)
    {
        // ESkill 취소
        bisEDrawing = false;
        GetWorld()->GetTimerManager().ClearTimer(RectangleUpdateTimerHandle);
        UE_LOG(LogTemp, Warning, TEXT("ESkill canceled."));
    }
    else
    {
        if (bIsQDrawing)
        {
            // QSkill 취소
            bIsQDrawing = false;
            GetWorld()->GetTimerManager().ClearTimer(CircleUpdateTimerHandle);
            UE_LOG(LogTemp, Warning, TEXT("QSkill canceled for ESkill."));
        }

        // ESkill 활성화
        bisEDrawing = true;

        // 카메라 위치와 방향 가져오기
        FVector CameraLocation;
        FRotator CameraRotation;
        GetController()->GetPlayerViewPoint(CameraLocation, CameraRotation);

        // 라인트레이스 시작점과 끝점 설정
        FVector Start = CameraLocation;
        FVector End = Start + (CameraRotation.Vector() * 6000.0f);

        // 충돌 파라미터 설정
        FHitResult HitResult;
        FCollisionQueryParams CollisionParams;
        CollisionParams.AddIgnoredActor(this);

        // 라인트레이스 실행
        bool bHit = GetWorld()->LineTraceSingleByChannel(HitResult, Start, End, ECC_Visibility, CollisionParams);

        if (bHit)
        {
            // 충돌 지점 저장
            CurrentImpactPoint = HitResult.ImpactPoint;

            // 사각형 업데이트 타이머 시작
            GetWorld()->GetTimerManager().SetTimer(RectangleUpdateTimerHandle, this, &APlayerCharacter::UpdateRectangle, 0.1f, true);
            UE_LOG(LogTemp, Warning, TEXT("ESkill activated."));
        }
    }
}

void APlayerCharacter::UpdateCircle()
{
    if (bIsQDrawing)
    {
        // 카메라 위치와 방향 가져오기
        FVector CameraLocation;
        FRotator CameraRotation;
        GetController()->GetPlayerViewPoint(CameraLocation, CameraRotation);

        // 라인트레이스 시작점과 끝점 설정
        FVector Start = CameraLocation;
        FVector End = Start + (CameraRotation.Vector() * 10000.0f); // 10,000 단위 거리

        // 충돌 파라미터 설정
        FHitResult HitResult;
        FCollisionQueryParams CollisionParams;
        CollisionParams.AddIgnoredActor(this); // 자기 자신은 무시

        // 라인트레이스 실행
        bool bHit = GetWorld()->LineTraceSingleByChannel(HitResult, Start, End, ECC_Visibility, CollisionParams);

        if (bHit)
        {
            // 충돌 지점 업데이트
            CurrentImpactPoint = HitResult.ImpactPoint;
			FVector Direction = (CurrentImpactPoint - GetActorLocation()).GetSafeNormal();
			CurrentImpactRot = Direction.Rotation();
            // 충돌 지점에 구 모양의 디버그 라인 표시
            DrawDebugSphere(GetWorld(), CurrentImpactPoint, 50.0f, 12, FColor::Green, false, 0.1f);
        }
    }
}

void APlayerCharacter::UpdateRectangle()
{
    if (bisEDrawing)
    {
        // 카메라 위치와 방향 가져오기
        FVector CameraLocation;
        FRotator CameraRotation;
        GetController()->GetPlayerViewPoint(CameraLocation, CameraRotation);

        // 라인트레이스 시작점과 끝점 설정
        FVector Start = CameraLocation;
        FVector End = Start + (CameraRotation.Vector() * 6000.0f); // 6,000 단위 거리

        // 충돌 파라미터 설정
        FHitResult HitResult;
        FCollisionQueryParams CollisionParams;
        CollisionParams.AddIgnoredActor(this); // 자기 자신은 무시

        // 라인트레이스 실행
        bool bHit = GetWorld()->LineTraceSingleByChannel(HitResult, Start, End, ECC_Visibility, CollisionParams);

        if (bHit)
        {
            // 충돌 지점 업데이트
            CurrentImpactPoint = HitResult.ImpactPoint;

            // 방향 업데이트
            FVector Direction = (CurrentImpactPoint - GetActorLocation()).GetSafeNormal();
            CurrentImpactRot = Direction.Rotation();

            // 바닥 높이를 유지하도록 Z 좌표를 고정
            CurrentImpactPoint.Z = HitResult.Location.Z;

            // 사각형의 중심 좌표
            FVector Center = CurrentImpactPoint;

            // 사각형의 크기와 방향 설정
            float RectangleWidth = 800.0f;  // 사각형의 너비
            float RectangleHeight = 200.0f; // 사각형의 높이
            FVector Forward = CameraRotation.Vector();
            FVector Right = FVector::CrossProduct(Forward, FVector::UpVector).GetSafeNormal();

            // 사각형의 네 모서리 좌표 계산 (Z 고정)
            FVector TopLeft = Center - (Right * RectangleWidth / 2) + (Forward * RectangleHeight / 2);
            FVector TopRight = Center + (Right * RectangleWidth / 2) + (Forward * RectangleHeight / 2);
            FVector BottomLeft = Center - (Right * RectangleWidth / 2) - (Forward * RectangleHeight / 2);
            FVector BottomRight = Center + (Right * RectangleWidth / 2) - (Forward * RectangleHeight / 2);

            // 각 꼭짓점의 Z 좌표를 충돌 지점의 Z로 고정
            TopLeft.Z = CurrentImpactPoint.Z;
            TopRight.Z = CurrentImpactPoint.Z;
            BottomLeft.Z = CurrentImpactPoint.Z;
            BottomRight.Z = CurrentImpactPoint.Z;

            // 사각형을 디버그 라인으로 그리기
            DrawDebugLine(GetWorld(), TopLeft, TopRight, FColor::Blue, false, 0.1f, 0, 2.0f);
            DrawDebugLine(GetWorld(), TopRight, BottomRight, FColor::Blue, false, 0.1f, 0, 2.0f);
            DrawDebugLine(GetWorld(), BottomRight, BottomLeft, FColor::Blue, false, 0.1f, 0, 2.0f);
            DrawDebugLine(GetWorld(), BottomLeft, TopLeft, FColor::Blue, false, 0.1f, 0, 2.0f);
        }
    }
}

void APlayerCharacter::GetFireTargetLocation()
{
	// 카메라가 유효한지 확인 (Camera로 수정)
	if (!Camera)
	{
		FireLocation = GetActorForwardVector() * TraceDistance;
		return;
	}

	FVector Start = Camera->GetComponentLocation();
	FVector End = Start + (Camera->GetForwardVector() * TraceDistance);

	FHitResult HitResult;
	FCollisionQueryParams Params(SCENE_QUERY_STAT(FireTrace), false, this);

	bool bHit = GetWorld()->LineTraceSingleByChannel(
		HitResult,
		Start,
		End,
		ECC_Visibility,
		Params
	);

	FireLocation = bHit ? HitResult.ImpactPoint : End;

	// 캐릭터의 Yaw를 카메라 Yaw에 맞춤
	FRotator ActorRot = GetActorRotation();
	FRotator ControlRot = GetControlRotation();
	SetActorRotation(FRotator(ActorRot.Pitch, ControlRot.Yaw, ActorRot.Roll));
}

void APlayerCharacter::use_skill(unsigned short skill_id, char skill_type, FVector v, bool is_left) {
	switch (skill_type) {
	case SKILL_WIND_CUTTER:
	case SKILL_FIRE_BALL:
	case SKILL_STONE_WAVE:
		m_skill_id = skill_id;
		FireLocation = v;
		bIsLeft = is_left;
		ComboStart();
		break;

	case SKILL_WIND_TORNADO:
	case SKILL_STONE_SKILL:
		m_skill_id = skill_id;
		CurrentImpactPoint = v;
		bIsLeft = is_left;
		SkillAttack();
		bIsQDrawing = false;
		GetWorld()->GetTimerManager().ClearTimer(CircleUpdateTimerHandle);
		break;

	case SKILL_ICE_ARROW:m_skill_id = skill_id;
		FireLocation = v;
		bIsLeft = is_left;
		ShootIceArrow();
		break;
	}
}

void APlayerCharacter::use_skill(unsigned short skill_id, char skill_type, FVector v, FRotator r, bool is_left) {
	switch (skill_type) {
	case SKILL_FIRE_WALL:
		m_skill_id = skill_id;
		CurrentImpactPoint = v;
		CurrentImpactRot = r;
		bIsLeft = is_left;
		SkillAttack();
		bIsQDrawing = false;
		GetWorld()->GetTimerManager().ClearTimer(CircleUpdateTimerHandle);
		break;
	}
}

void APlayerCharacter::change_element() {
	player_change_element_packet p;
	p.packet_size = sizeof(player_change_element_packet);
	p.packet_type = C2H_PLAYER_CHANGE_ELEMENT_PACKET;
	p.player_id = m_id;
	p.is_left = true;

	switch (LeftClassType) {
	case EClassType::CT_Wind:
		UE_LOG(LogTemp, Warning, TEXT("Class changed to Fire"));
		ChangeClass(EClassType::CT_Fire, true);
		p.element_type = static_cast<char>(EClassType::CT_Fire);
		break;

	case EClassType::CT_Fire:
		UE_LOG(LogTemp, Warning, TEXT("Class changed to Stone"));
		ChangeClass(EClassType::CT_Stone, true);
		p.element_type = static_cast<char>(EClassType::CT_Stone);
		break;

	case EClassType::CT_Stone:
		UE_LOG(LogTemp, Warning, TEXT("Class changed to Ice"));
		ChangeClass(EClassType::CT_Ice, true);
		p.element_type = static_cast<char>(EClassType::CT_Ice);
		break;

	case EClassType::CT_Ice:
		UE_LOG(LogTemp, Warning, TEXT("Class changed to Wind"));
		ChangeClass(EClassType::CT_Wind, true);
		p.element_type = static_cast<char>(EClassType::CT_Wind);
		break;

	default:
		UE_LOG(LogTemp, Warning, TEXT("Unknown class type"));
		break;
	}

	do_send(&p);
}

void APlayerCharacter::change_element(char element_type, bool is_left) {
	ChangeClass(static_cast<EClassType>(element_type), is_left);
}
		
void APlayerCharacter::rotate(float yaw) {
	FRotator NewRotation = GetActorRotation();
	NewRotation.Yaw = yaw;
	SetActorRotation(NewRotation);
}

void APlayerCharacter::do_send(void* buff) {
	EXP_OVER* o = new EXP_OVER;
	unsigned char packet_size = reinterpret_cast<unsigned char*>(buff)[0];
	memcpy(o->m_buffer, buff, packet_size);
	o->m_wsabuf[0].len = packet_size;

	DWORD send_bytes;
	auto ret = WSASend(g_c_socket, o->m_wsabuf, 1, &send_bytes, 0, &(o->m_over), send_callback);
	if (ret == SOCKET_ERROR) {
		if (WSAGetLastError() != WSA_IO_PENDING) {
			delete o;
			return;
		}
	}
}

void APlayerCharacter::UpdateUI()
{
    if (CharacterWidget)
    {
        CharacterWidget->UpdateHpBar(playerCurrentHp, playerMaxHp);
        CharacterWidget->UpdateMpBar(playerCurrentMp, playerMaxMp);
    }
}

void APlayerCharacter::PlayFootstepSound()
{
    if (FootstepSounds.Num() > 0)
    {
        // 현재 인덱스의 효과음을 재생
        if (FootstepSounds[CurrentFootstepIndex])
        {
            UGameplayStatics::PlaySoundAtLocation(this, FootstepSounds[CurrentFootstepIndex], GetActorLocation());
        }

        // 인덱스를 증가시키고, 배열 크기를 초과하면 0으로 초기화
        CurrentFootstepIndex = (CurrentFootstepIndex + 1) % FootstepSounds.Num();
    }
}