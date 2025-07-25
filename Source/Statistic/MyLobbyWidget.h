// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Components/EditableText.h"
#include "MyLobbyWidget.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnStartPressedDelegate, const FString&, EnteredIP);

UCLASS()
class STATISTIC_API UMyLobbyWidget : public UUserWidget
{
	GENERATED_BODY()
public:
	UPROPERTY(meta = (BindWidget))
	UEditableText* IPText;

	UPROPERTY(meta = (BindWidget))
	class UButton* StartButton;

	UPROPERTY(BlueprintAssignable)
	FOnStartPressedDelegate OnStartPressed;

protected:
	virtual bool Initialize() override;

	// 클릭 처리
	UFUNCTION()
	void HandleStartClicked();

	// IP값 가져오기 함수
	FString GetEnteredIP() const;
};
