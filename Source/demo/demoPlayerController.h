// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "demoPlayerController.generated.h"

class UInputMappingContext;
class UUserWidget;
class UDemoSettingsWidget;
class UInputAction;

/**
 *  Basic PlayerController class for a third person game
 *  Manages input mappings and the Esc settings menu
 */
UCLASS(abstract)
class AdemoPlayerController : public APlayerController
{
	GENERATED_BODY()

protected:

	/** Input Mapping Contexts */
	UPROPERTY(EditAnywhere, Category ="Input|Input Mappings")
	TArray<UInputMappingContext*> DefaultMappingContexts;

	/** Input Mapping Contexts */
	UPROPERTY(EditAnywhere, Category="Input|Input Mappings")
	TArray<UInputMappingContext*> MobileExcludedMappingContexts;

	/** Mobile controls widget to spawn */
	UPROPERTY(EditAnywhere, Category="Input|Touch Controls")
	TSubclassOf<UUserWidget> MobileControlsWidgetClass;

	/** Pointer to the mobile controls widget */
	UPROPERTY()
	TObjectPtr<UUserWidget> MobileControlsWidget;

	/** If true, the player will use UMG touch controls even if not playing on mobile platforms */
	UPROPERTY(EditAnywhere, Config, Category = "Input|Touch Controls")
	bool bForceTouchControls = false;

	UPROPERTY()
	TObjectPtr<UDemoSettingsWidget> SettingsWidget;

	UPROPERTY()
	TObjectPtr<UInputMappingContext> MenuMappingContext;

	UPROPERTY()
	TObjectPtr<UInputAction> ToggleMenuAction;

	UPROPERTY()
	TObjectPtr<UInputAction> ApplyMenuAction;

	/** Gameplay initialization */
	virtual void BeginPlay() override;

	/** Input mapping context setup */
	virtual void SetupInputComponent() override;

	/** Returns true if the player should use UMG touch controls */
	bool ShouldUseTouchControls() const;

	void ToggleSettingsMenu();
	void OpenSettingsMenu();
	void CloseSettingsMenu();
	void ApplySettingsFromShortcut();
	void ReturnToMainMenu();

	UPROPERTY(EditAnywhere, Category = "Main Menu")
	FString MainMenuLevelPath = TEXT("/Game/Maps/Lvl_MainMenu");

	UPROPERTY(EditAnywhere, Category = "Main Menu")
	FString MainMenuGameModeOption = TEXT("game=/Script/demo.DemoMainMenuGameMode");

	UFUNCTION()
	void HandleSettingsClosed();

	UFUNCTION()
	void HandleReturnToMainMenu();
};
