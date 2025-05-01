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
#include "MyWeapon.h"
#include "MyFireWeapon.h"
#include "Enums.h"

#include "SESSION.h"

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
		bUseControllerRotationYaw = true;
		bUseControllerRotationRoll = false;

		// 폰의 컨트롤 회전 사용
		SpringArm->bUsePawnControlRotation = true;
		// 움직임에 따른 회전 On
		GetCharacterMovement()->bOrientRotationToMovement = false;
		// 점프 높이 설정
        GetCharacterMovement()->JumpZVelocity = 400.0f; // 원하는 값으로 설정
		GetCharacterMovement()->MaxWalkSpeed = 300.0f;
		 // Member Variable 초기화
	}
	{
		bIsDash = false;
	}

    // 기본 클래스 타입 설정
    ClassType = EClassType::CT_Fire;
    // 초기 캐싱된 데이터 설정
    CurrentMontage = nullptr;
    CurrentComboData = nullptr;

	static ConstructorHelpers::FClassFinder<AMyWeapon> FireWeaponBPRef(TEXT("/Game/Weapon/BP_FIreWeapon.BP_FIreWeapon_C"));
    if (FireWeaponBPRef.Succeeded())
    {
        FireWeaponBP = FireWeaponBPRef.Class;
    }

    // WindWeaponBP 초기화
    static ConstructorHelpers::FClassFinder<AMyWeapon> WindWeaponBPRef(TEXT("/Game/Weapon/BP_WindWeapon.BP_WindWeapon_C"));
    if (WindWeaponBPRef.Succeeded())
    {
        WindWeaponBP = WindWeaponBPRef.Class;
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

	{
	ChangeClass(EClassType::CT_Fire);
	}

    // 초기 캐싱된 데이터 업데이트
    UpdateCachedData();
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
        GetCharacterMovement()->MaxWalkSpeed = 300.0f;
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
            GetCharacterMovement()->MaxWalkSpeed = 300.0f;
        }
    }

    // Movement에 값 전달 (방향, 이동량)
    AddMovementInput(ForwardDirection, MovementVector.X);
    AddMovementInput(RightDirection, MovementVector.Y);

	// Player Vector Packet Send
	player_vector_packet p;
	p.packet_size = sizeof(player_vector_packet);
	p.packet_type = C2H_PLAYER_VECTOR_PACKET;
	p.id = g_id;
	p.x = g_x; p.y = g_y; p.z = g_z;
	p.dx = MovementVector.X; p.dy = MovementVector.Y;
	do_send(&p);
	UE_LOG(LogTemp, Warning, TEXT("[Client] Send Packet to Host"));
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
	if(!CheckBackMove)
	{
		bIsDash = true;
	}
}

void APlayerCharacter::DashEnd()
{
	bIsDash = false;
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

		CurrentComboCount = FMath::Clamp(CurrentComboCount + 1, 1, CurrentComboData->MaxComboCount);

		UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();
		if (AnimInstance && CurrentMontage)
		{
			// 현재 섹션 이름 얻기
			FName CurrentSectionName = AnimInstance->Montage_GetCurrentSection(CurrentMontage);

			// 다음 섹션 이름 생성
			FName NextSectionName = *FString::Printf(TEXT("%s%d"), *CurrentComboData->SectionPrefix, CurrentComboCount);
			UE_LOG(LogTemp, Warning, TEXT("Current: %s → Next: %s"), *CurrentSectionName.ToString(), *NextSectionName.ToString());

			// 현재 섹션 끝나면 다음 섹션으로 자연스럽게 블렌드되도록 설정
			AnimInstance->Montage_SetNextSection(CurrentSectionName, NextSectionName, CurrentMontage);

			// 다음 콤보를 위한 입력 초기화 및 타이머 재설정
			SetComboTimer();
			bHasComboInput = false;
		}
	}
}

void APlayerCharacter::BaseAttackCheck()
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

void APlayerCharacter::ChangeClass(EClassType NewClassType)
{
    if (ClassType != NewClassType)
    {
        ClassType = NewClassType;
        UpdateCachedData(); // 클래스 변경 시 캐싱된 데이터 업데이트
        UE_LOG(LogTemp, Warning, TEXT("Class Type Changed to: %d"), static_cast<int32>(ClassType));
    }
}

void APlayerCharacter::UpdateCachedData()
{
    switch (ClassType)
    {
    case EClassType::CT_Wind:
        CurrentMontage = WindComboMontage;
        CurrentComboData = WindComboData;
		WeaponClass = WindWeaponBP;
		CheckAnimBone = 1;
        break;

    case EClassType::CT_Stone:
        CurrentMontage = StoneComboMontage;
        CurrentComboData = StoneComboData;
		CheckAnimBone = 0;
        break;
	case EClassType::CT_Fire:
        CurrentMontage = FireComboMontage;
        CurrentComboData = FireComboData;
		WeaponClass = FireWeaponBP;
		CheckAnimBone = 1;
        break;
    default:
        CurrentMontage = nullptr;
        CurrentComboData = nullptr;
        break;
    }
	if (GetWorld())
		{
			CurrentWeapon = Cast<AMyWeapon>(GetWorld()->SpawnActor<AMyWeapon>(WeaponClass));

			
			UE_LOG(LogTemp, Warning, TEXT("WeaponClass Load: %s"), CurrentWeapon ? TEXT("Success") : TEXT("Fail"));

			if (CurrentWeapon)
			{
				UE_LOG(LogTemp, Warning, TEXT("Weapon Spawned"));
				EquipWeapon(CurrentWeapon);
			}
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

void APlayerCharacter::EquipWeapon(AMyWeapon* Weapon)
{
	Weapon->EquipWeapon(this);
}

void APlayerCharacter::do_send(void* buff) {
	EXP_OVER* o = new EXP_OVER;
	unsigned char packet_size = reinterpret_cast<unsigned char*>(buff)[0];
	memcpy(o->m_buffer, buff, packet_size);
	o->m_wsabuf[0].len = packet_size;
	DWORD send_bytes;
	WSASend(g_h_socket, o->m_wsabuf, 1, &send_bytes, 0, &(o->m_over), NULL);
}