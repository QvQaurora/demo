// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "DemoMainMenuWidget.generated.h"

class UButton;

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnDemoMainMenuStartGame);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnDemoMainMenuOpenSettings);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnDemoMainMenuQuitGame);

/**
 * Title / start menu: Start Game + Settings + Quit.
 */
UCLASS()
class UDemoMainMenuWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	UPROPERTY(BlueprintAssignable, Category = "Main Menu")
	FOnDemoMainMenuStartGame OnStartGame;

	UPROPERTY(BlueprintAssignable, Category = "Main Menu")
	FOnDemoMainMenuOpenSettings OnOpenSettings;

	UPROPERTY(BlueprintAssignable, Category = "Main Menu")
	FOnDemoMainMenuQuitGame OnQuitGame;

	UDemoMainMenuWidget(const FObjectInitializer& ObjectInitializer);

protected:
	virtual TSharedRef<SWidget> RebuildWidget() override;
	virtual void NativeConstruct() override;

	void BuildLayout();

	UFUNCTION()
	void HandleStartClicked();

	UFUNCTION()
	void HandleSettingsClicked();

	UFUNCTION()
	void HandleQuitClicked();

	UPROPERTY()
	TObjectPtr<UButton> StartButton = nullptr;

	UPROPERTY()
	TObjectPtr<UButton> SettingsButton = nullptr;

	UPROPERTY()
	TObjectPtr<UButton> QuitButton = nullptr;

	bool bLayoutBuilt = false;
};
