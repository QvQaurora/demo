// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "DemoMainMenuPlayerController.generated.h"

class UDemoMainMenuWidget;
class UDemoSettingsWidget;

/**
 * PlayerController for the title screen. Shows the start menu and
 * loads the first gameplay level only after Start Game is pressed.
 */
UCLASS()
class ADemoMainMenuPlayerController : public APlayerController
{
	GENERATED_BODY()

public:
	ADemoMainMenuPlayerController();

protected:
	virtual void BeginPlay() override;

	/** Soft object path without class suffix, e.g. /Game/Maps/Lvl_ThirdPerson */
	UPROPERTY(EditAnywhere, Category = "Main Menu")
	FString FirstLevelPath = TEXT("/Game/Maps/Lvl_ThirdPerson");

	/** Soft path option so the gameplay level uses the third-person GameMode. */
	UPROPERTY(EditAnywhere, Category = "Main Menu")
	FString FirstLevelGameModeOption = TEXT("game=/Game/ThirdPerson/Blueprints/BP_ThirdPersonGameMode.BP_ThirdPersonGameMode_C");

	UPROPERTY()
	TObjectPtr<UDemoMainMenuWidget> MainMenuWidget;

	UPROPERTY()
	TObjectPtr<UDemoSettingsWidget> SettingsWidget;

	void ShowMainMenu();
	void ApplyMenuInputMode(UUserWidget* FocusWidget);

	UFUNCTION()
	void HandleStartGame();

	UFUNCTION()
	void HandleOpenSettings();

	UFUNCTION()
	void HandleQuitGame();

	UFUNCTION()
	void HandleSettingsClosed();
};
