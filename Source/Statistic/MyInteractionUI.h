#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Components/Image.h"
#include "Components/EditableTextBox.h"
#include "Animation/WidgetAnimation.h"
#include "MyInteractionUI.generated.h"

UCLASS()
class STATISTIC_API UMyInteractionUI : public UUserWidget
{
	GENERATED_BODY()
	
public:
	// F 키 이미지
	UPROPERTY(meta = (BindWidget))
	UImage* FKeyImage;

	// 입력 가능한 텍스트 박스
	UPROPERTY(meta = (BindWidget))
	UEditableTextBox* FKeyTextBox;

	// 시작 애니메이션
	UPROPERTY(Transient, meta = (BindWidgetAnim))
	UWidgetAnimation* StartAnim;

	// 시작 시 애니메이션 재생
	virtual void NativeConstruct() override;
	
	UFUNCTION(BlueprintCallable)
	void PlayStartAnimation();
	
};
