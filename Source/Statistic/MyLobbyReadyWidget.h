#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Components/Image.h"
#include "Components/Button.h"
#include "Components/TextBlock.h"
#include "Enums.h"
#include "MyLobbyReadyWidget.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnOutPressed);

UCLASS()
class STATISTIC_API UMyLobbyReadyWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	// 이미지: Number1OnOff ~ Number4OnOff
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UImage> Number1OnOff;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UImage> Number2OnOff;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UImage> Number3OnOff;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UImage> Number4OnOff;

	// 버튼: StartButton, OutButton
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UButton> StartButton;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UButton> OutButton;

	// 텍스트 블록: Number1 ~ Number4
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> Number1;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> Number2;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> Number3;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> Number4;

	// 이미지: Number1Left ~ Number4Left, Number1Right ~ Number4Right
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UImage> Number1Left;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UImage> Number1Right;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UImage> Number2Left;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UImage> Number2Right;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UImage> Number3Left;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UImage> Number3Right;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UImage> Number4Left;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UImage> Number4Right;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UImage> MainLeft;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UImage> MainRight;

	// 버튼: ChangeLeft, ChangeRight
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UButton> ChangeLeft;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UButton> ChangeRight;

	// 텍스처: 사람 그림 및 속성 그림 (불, 얼음, 바람, 땅)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="SkillIcon")
	TObjectPtr<UTexture2D> CharacterTexture;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="SkillIcon")
	TObjectPtr<UTexture2D> AttributeFireTexture;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="SkillIcon")
	TObjectPtr<UTexture2D> AttributeIceTexture;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="SkillIcon")
	TObjectPtr<UTexture2D> AttributeWindTexture;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="SkillIcon")
	TObjectPtr<UTexture2D> AttributeStoneTexture;

	// 버튼 바인딩용 함수들
	UFUNCTION()
	void OnStartButtonClicked();

	UFUNCTION()
	void OnOutButtonClicked();

	UFUNCTION()
	void OnChangeLeftClicked();

	UFUNCTION()
	void OnChangeRightClicked();

	virtual void NativeConstruct() override;

	// 현재 선택된 속성
	EClassType CurrentClassTypeLeft = EClassType::CT_Wind;
	EClassType CurrentClassTypeRight = EClassType::CT_Wind;

	void UpdateImageByClassType(EClassType ClassType, UImage* ImageWidget);

public:
    UPROPERTY(BlueprintAssignable, Category="Events")
    FOnOutPressed OnOutPressed;

};
