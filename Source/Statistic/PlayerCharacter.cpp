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
	bIsDash = true;
	GetCharacterMovement()->MaxWalkSpeed = 600.0f;
}

void APlayerCharacter::DashEnd()
{
	bIsDash = false;
	GetCharacterMovement()->MaxWalkSpeed = 300.0f;
}	