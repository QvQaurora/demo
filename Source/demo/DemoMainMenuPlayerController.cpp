// Copyright Epic Games, Inc. All Rights Reserved.

#include "DemoMainMenuPlayerController.h"
#include "DemoGameUserSettings.h"
#include "DemoMainMenuWidget.h"
#include "DemoSettingsWidget.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetSystemLibrary.h"

ADemoMainMenuPlayerController::ADemoMainMenuPlayerController()
{
	bShowMouseCursor = true;
	bEnableClickEvents = true;
	bEnableMouseOverEvents = true;
}

void ADemoMainMenuPlayerController::BeginPlay()
{
	Super::BeginPlay();

	if (UDemoGameUserSettings* Settings = UDemoGameUserSettings::GetDemoGameUserSettings())
	{
		Settings->LoadSettings();
		Settings->ApplySettings(false);
	}

	if (IsLocalPlayerController())
	{
		ShowMainMenu();
	}
}

void ADemoMainMenuPlayerController::ShowMainMenu()
{
	if (!MainMenuWidget)
	{
		MainMenuWidget = CreateWidget<UDemoMainMenuWidget>(this, UDemoMainMenuWidget::StaticClass());
		if (MainMenuWidget)
		{
			MainMenuWidget->OnStartGame.AddDynamic(this, &ADemoMainMenuPlayerController::HandleStartGame);
			MainMenuWidget->OnOpenSettings.AddDynamic(this, &ADemoMainMenuPlayerController::HandleOpenSettings);
			MainMenuWidget->OnQuitGame.AddDynamic(this, &ADemoMainMenuPlayerController::HandleQuitGame);
		}
	}

	if (!MainMenuWidget)
	{
		return;
	}

	if (!MainMenuWidget->IsInViewport())
	{
		MainMenuWidget->AddToViewport(50);
	}

	ApplyMenuInputMode(MainMenuWidget);
}

void ADemoMainMenuPlayerController::ApplyMenuInputMode(UUserWidget* FocusWidget)
{
	bShowMouseCursor = true;
	FInputModeUIOnly InputMode;
	if (FocusWidget)
	{
		InputMode.SetWidgetToFocus(FocusWidget->TakeWidget());
	}
	InputMode.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);
	SetInputMode(InputMode);
	if (FocusWidget)
	{
		FocusWidget->SetKeyboardFocus();
	}
}

void ADemoMainMenuPlayerController::HandleStartGame()
{
	if (SettingsWidget && SettingsWidget->IsInViewport())
	{
		SettingsWidget->RemoveFromParent();
	}
	if (MainMenuWidget)
	{
		MainMenuWidget->RemoveFromParent();
	}

	// Clear UI-only capture before travel so the next level can take game input.
	bShowMouseCursor = false;
	SetInputMode(FInputModeGameOnly());

	const FName LevelName(*FirstLevelPath);
	UGameplayStatics::OpenLevel(this, LevelName, true, FirstLevelGameModeOption);
}

void ADemoMainMenuPlayerController::HandleOpenSettings()
{
	if (!SettingsWidget)
	{
		SettingsWidget = CreateWidget<UDemoSettingsWidget>(this, UDemoSettingsWidget::StaticClass());
		if (SettingsWidget)
		{
			SettingsWidget->OnClosed.AddDynamic(this, &ADemoMainMenuPlayerController::HandleSettingsClosed);
		}
	}

	if (!SettingsWidget)
	{
		return;
	}

	SettingsWidget->SetShowReturnToMainMenu(false);

	if (!SettingsWidget->IsInViewport())
	{
		SettingsWidget->AddToViewport(100);
	}

	SettingsWidget->CaptureStateForCancel();
	ApplyMenuInputMode(SettingsWidget);
}

void ADemoMainMenuPlayerController::HandleQuitGame()
{
	UKismetSystemLibrary::QuitGame(this, this, EQuitPreference::Quit, false);
}

void ADemoMainMenuPlayerController::HandleSettingsClosed()
{
	if (SettingsWidget)
	{
		SettingsWidget->RemoveFromParent();
	}
	ShowMainMenu();
}
