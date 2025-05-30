// Fill out your copyright notice in the Description page of Project Settings.


#include "DamagePopupActor.h"
#include "Components/WidgetComponent.h"
#include "DamageWidget.h"
#include "Components/TextBlock.h"

// Sets default values
ADamagePopupActor::ADamagePopupActor()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	widgetComp = CreateDefaultSubobject<UWidgetComponent>(TEXT("WidgetComp"));
	RootComponent = widgetComp;

	widgetComp->SetWidgetSpace(EWidgetSpace::Screen);
	widgetComp->SetDrawAtDesiredSize(true);
	widgetComp->SetUsingAbsoluteRotation(true);

}

// Called when the game starts or when spawned
void ADamagePopupActor::BeginPlay()
{
	Super::BeginPlay();
	GetWorldTimerManager().SetTimerForNextTick([this]()
	{
		SetLifeSpan(1.5f);
	});
}

// Called every frame
void ADamagePopupActor::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	APlayerController* PC = GetWorld()->GetFirstPlayerController();
	if (!PC) return;

	FVector CamLocation;
	FRotator CamRotation;
	PC->GetPlayerViewPoint(CamLocation, CamRotation);

	FVector ToCamera = CamLocation - GetActorLocation();
	FRotator LookAtRotation = FRotationMatrix::MakeFromX(ToCamera).Rotator();

	// Z 축만 회전하게 (카메라를 바라보되 수직은 고정하고 싶을 경우)
	LookAtRotation.Pitch = 0.0f;
	LookAtRotation.Roll = 0.0f;

	widgetComp->SetWorldRotation(LookAtRotation);
}

void ADamagePopupActor::InitDamage(float damage, EClassType Type)
{
	damageWidgetInstance = Cast<UDamageWidget>(widgetComp->GetUserWidgetObject());
	if(damageWidgetInstance)
	{
		damageWidgetInstance->PlayNormalDamageAnimation(damage , Type);
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("DamageWidgetInstance is null"));
	}
}
