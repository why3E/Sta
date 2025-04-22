// Fill out your copyright notice in the Description page of Project Settings.


#include "PlayerCharacter.h"
#include "Components/CapsuleComponent.h"
#include "Camera/CameraComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "GameFramework/CharacterMovementComponent.h"

#include "InputAction.h"
#include "InputMappingContext.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "MMComboActionData.h" // Include the header for UMMComboActionData

APlayerCharacter::APlayerCharacter()
{
	// Collision 설정
	{
		GetCapsuleComponent()->InitCapsuleSize(35.0f, 90.0f);
	}

	// Mesh 설정
	{
		// Load
		static ConstructorHelpers::FObjectFinder<USkeletalMesh> SkeletalMeshRef(TEXT("/Game/MilitaryMercenaryBandit/Meshes/SK_Bandit.SK_Bandit"));
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
    }

	// Setting (기본적으로 원하는 기본 이동을 위한 캐릭터 설정)
	{
		// 컨트롤러의 Rotation에 영향 X
		bUseControllerRotationPitch = false;
		bUseControllerRotationYaw = false;
		bUseControllerRotationRoll = false;

		// 폰의 컨트롤 회전 사용
		SpringArm->bUsePawnControlRotation = true;
		// 움직임에 따른 회전 On
		GetCharacterMovement()->bOrientRotationToMovement = true;

		// 점프 높이 설정
        GetCharacterMovement()->JumpZVelocity = 400.0f; // 원하는 값으로 설정
		GetCharacterMovement()->MaxWalkSpeed = 300.0f;
		 // Member Variable 초기화
	}
	{
		bIsDash = false;
	}
}

void APlayerCharacter::BeginPlay()
{
	Super::BeginPlay();

	APlayerController* PlayerController = Cast<APlayerController>(GetController());
	if (PlayerController && IMC_Basic)
	{
    	// 서브시스템 불러오기
		if (UEnhancedInputLocalPlayerSubsystem* SubSystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(PlayerController->GetLocalPlayer()))
		{
			// 매핑 컨텍스트 추가
			SubSystem->AddMappingContext(IMC_Basic, 0);
			// 입력 시작
			EnableInput(PlayerController);
		}
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

	EnhancedInputComponent->BindAction(IA_BasicAttack, ETriggerEvent::Triggered, this, &APlayerCharacter::BasicAttack);
}

void APlayerCharacter::BasicMove(const FInputActionValue& Value)
{
	// 입력받은 Value로부터 MovementVector 가져오기
	FVector2D MovementVector = Value.Get<FVector2D>();

	// 컨트롤러의 회전 중 Yaw(Z)를 가져와 저장
	const FRotator Rotation = Controller->GetControlRotation();
	const FRotator YawRotation(0, Rotation.Yaw, 0);

	// 회전(Yaw)을 기반으로 전방 및 오른쪽 방향을 받아오기 (X : 전방, Y : 오른쪽)
	const FVector ForwardDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);
	const FVector RightDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);

	// Movement에 값 전달 (방향, 이동량)
	AddMovementInput(ForwardDirection, MovementVector.X);
	AddMovementInput(RightDirection, MovementVector.Y);
}

void APlayerCharacter::BasicLook(const FInputActionValue& Value)
{
	// 입력받은 Value로부터 LookVector 가져오기
	FVector2D LookVector = Value.Get<FVector2D>();
	
	// Controller에 값 전달
	AddControllerYawInput(LookVector.X);
	AddControllerPitchInput(LookVector.Y);
}

void APlayerCharacter::StartJump()
{
    bPressedJump = true;
}

void APlayerCharacter::StopJump()
{
    bPressedJump = false;
}

void APlayerCharacter::DashStart()
{
	UE_LOG(LogTemp, Warning, TEXT("DashStart!"));
	bIsDash = true;
	GetCharacterMovement()->MaxWalkSpeed = 600.0f;
}

void APlayerCharacter::DashEnd()
{
	bIsDash = false;
	GetCharacterMovement()->MaxWalkSpeed = 300.0f;
}	

void APlayerCharacter::BasicAttack()
{
	UE_LOG(LogTemp, Warning, TEXT("Basic Attack!"));

	if (CurrentComboCount == 0)
	{
		ComboStart();
		return;
	}

	// 중간 입력 체크
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

void APlayerCharacter::ComboStart()
{
    CurrentComboCount = 1;
    const float AttackSpeedRate = 2.0f;

    UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();
    if (!AnimInstance)
    {
        UE_LOG(LogTemp, Error, TEXT("AnimInstance is nullptr!"));
        return;
    }

    if (!BasicComboMontage)
    {
        UE_LOG(LogTemp, Error, TEXT("BasicComboMontage is nullptr!"));
        return;
    }

    // 몽타주 재생
    float MontageLength = AnimInstance->Montage_Play(BasicComboMontage, AttackSpeedRate);
    if (MontageLength > 0.0f)
    {
        UE_LOG(LogTemp, Warning, TEXT("Montage_Play succeeded! Montage Length: %f"), MontageLength);

        // 몽타주 재생 종료 바인딩
        FOnMontageEnded EndDelegate;
        EndDelegate.BindUObject(this, &APlayerCharacter::ComboEnd);

        // BasicComboMontage가 종료되면 EndDelegate에 연동된 ComboEnd 함수 호출
        AnimInstance->Montage_SetEndDelegate(EndDelegate, BasicComboMontage);

        // 타이머 초기화
        ComboTimerHandle.Invalidate();
        // 타이머 설정
        SetComboTimer();
    }
    else
    {
        UE_LOG(LogTemp, Error, TEXT("Montage_Play failed!"));
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
	UE_LOG(LogTemp, Warning, TEXT("ComboCheck"));

	ComboTimerHandle.Invalidate();

	if (bHasComboInput)
	{
		UE_LOG(LogTemp, Warning, TEXT("ComboCheck ok "));

		CurrentComboCount = FMath::Clamp(CurrentComboCount + 1, 1, BasicComboData->MaxComboCount);

		UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();
		if (AnimInstance && BasicComboMontage)
		{
			// 현재 섹션 이름 얻기
			FName CurrentSectionName = AnimInstance->Montage_GetCurrentSection(BasicComboMontage);

			// 다음 섹션 이름 생성
			FName NextSectionName = *FString::Printf(TEXT("%s%d"), *BasicComboData->SectionPrefix, CurrentComboCount);
			UE_LOG(LogTemp, Warning, TEXT("Current: %s → Next: %s"), *CurrentSectionName.ToString(), *NextSectionName.ToString());

			// 현재 섹션 끝나면 다음 섹션으로 자연스럽게 블렌드되도록 설정
			AnimInstance->Montage_SetNextSection(CurrentSectionName, NextSectionName, BasicComboMontage);

			// 다음 콤보를 위한 입력 초기화 및 타이머 재설정
			SetComboTimer();
			bHasComboInput = false;
		}
	}
}

void APlayerCharacter::SetComboTimer()
{
	
	UE_LOG(LogTemp, Warning, TEXT("SetComboTimer"));
	int32 ComboIndex = CurrentComboCount - 1;

	// 인덱스가 유효한지 체크
	if (BasicComboData->ComboFrame.IsValidIndex(ComboIndex))
	{
		// TODO : 공격 속도가 추가되면 값 가져와 지정하기
		const float AttackSpeedRate = 2.0f;

		// 실제 콤보가 입력될 수 있는 시간 구하기
		float ComboAvailableTime = (BasicComboData->ComboFrame[ComboIndex] / BasicComboData->FrameRate) / AttackSpeedRate;

		// 타이머 설정하기
		if (ComboAvailableTime > 0.0f)
		{
			// ComboAvailableTime시간이 지나면 ComboCheck() 함수 호출
			GetWorld()->GetTimerManager().SetTimer(ComboTimerHandle, this, &APlayerCharacter::ComboCheck, ComboAvailableTime, false);
		}
	}
}