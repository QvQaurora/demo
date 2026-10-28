// Copyright Epic Games, Inc. All Rights Reserved.

#include "demoPlayerController.h"
#include "DemoSettingsWidget.h"
#include "DemoGameUserSettings.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "Engine/LocalPlayer.h"
#include "InputAction.h"
#include "InputMappingContext.h"
#include "Blueprint/UserWidget.h"
#include "demo.h"
#include "Widgets/Input/SVirtualJoystick.h"
#include "Kismet/GameplayStatics.h"

void AdemoPlayerController::BeginPlay()
{
	Super::BeginPlay();

	if (UDemoGameUserSettings* Settings = UDemoGameUserSettings::GetDemoGameUserSettings())
	{
		Settings->LoadSettings();
		Settings->ApplySettings(false);
	}

	// Level travel from the main menu leaves UIOnly / NoCapture on the viewport.
	if (IsLocalPlayerController())
	{
		bShowMouseCursor = false;
		SetInputMode(FInputModeGameOnly());
	}

	// only spawn touch controls on local player controllers
	if (IsLocalPlayerController() && ShouldUseTouchControls())
	{
		MobileControlsWidget = CreateWidget<UUserWidget>(this, MobileControlsWidgetClass);

		if (MobileControlsWidget)
		{
			MobileControlsWidget->AddToPlayerScreen(0);
		}
		else
		{
			UE_LOG(Logdemo, Error, TEXT("Could not spawn mobile controls widget."));
		}
	}
}

void AdemoPlayerController::SetupInputComponent()
{
	Super::SetupInputComponent();

	if (!IsLocalPlayerController())
	{
		return;
	}

	if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(GetLocalPlayer()))
	{
		for (UInputMappingContext* CurrentContext : DefaultMappingContexts)
		{
			Subsystem->AddMappingContext(CurrentContext, 0);
		}

		if (!ShouldUseTouchControls())
		{
			for (UInputMappingContext* CurrentContext : MobileExcludedMappingContexts)
			{
				Subsystem->AddMappingContext(CurrentContext, 0);
			}
		}

		if (!ToggleMenuAction)
		{
			ToggleMenuAction = NewObject<UInputAction>(this, TEXT("ToggleMenuAction"));
			ToggleMenuAction->ValueType = EInputActionValueType::Boolean;
		}
		if (!ApplyMenuAction)
		{
			ApplyMenuAction = NewObject<UInputAction>(this, TEXT("ApplyMenuAction"));
			ApplyMenuAction->ValueType = EInputActionValueType::Boolean;
		}
		if (!MenuMappingContext)
		{
			MenuMappingContext = NewObject<UInputMappingContext>(this, TEXT("MenuMappingContext"));
			// Esc works in packaged builds; the editor steals Esc to stop PIE, so also bind P/O.
			MenuMappingContext->MapKey(ToggleMenuAction, EKeys::Escape);
			MenuMappingContext->MapKey(ToggleMenuAction, EKeys::P);
			MenuMappingContext->MapKey(ToggleMenuAction, EKeys::O);
			MenuMappingContext->MapKey(ApplyMenuAction, EKeys::Enter);
		}
		Subsystem->AddMappingContext(MenuMappingContext, 10);
	}

	if (UEnhancedInputComponent* EnhancedInput = Cast<UEnhancedInputComponent>(InputComponent))
	{
		EnhancedInput->BindAction(ToggleMenuAction, ETriggerEvent::Started, this, &AdemoPlayerController::ToggleSettingsMenu);
		EnhancedInput->BindAction(ApplyMenuAction, ETriggerEvent::Started, this, &AdemoPlayerController::ApplySettingsFromShortcut);
	}

	// Pause blocks Enhanced Input; keep Enter available while the menu is open.
	if (InputComponent)
	{
		FInputKeyBinding& EnterBinding = InputComponent->BindKey(EKeys::Enter, IE_Pressed, this, &AdemoPlayerController::ApplySettingsFromShortcut);
		EnterBinding.bExecuteWhenPaused = true;
	}
}

bool AdemoPlayerController::ShouldUseTouchControls() const
{
	return SVirtualJoystick::ShouldDisplayTouchInterface() || bForceTouchControls;
}

void AdemoPlayerController::ToggleSettingsMenu()
{
	if (SettingsWidget && SettingsWidget->IsInViewport())
	{
		CloseSettingsMenu();
	}
	else
	{
		OpenSettingsMenu();
	}
}

void AdemoPlayerController::OpenSettingsMenu()
{
	if (!IsLocalPlayerController())
	{
		return;
	}

	if (!SettingsWidget)
	{
		SettingsWidget = CreateWidget<UDemoSettingsWidget>(this, UDemoSettingsWidget::StaticClass());
		if (SettingsWidget)
		{
			SettingsWidget->OnClosed.AddDynamic(this, &AdemoPlayerController::HandleSettingsClosed);
			SettingsWidget->OnReturnToMainMenu.AddDynamic(this, &AdemoPlayerController::HandleReturnToMainMenu);
		}
	}

	if (!SettingsWidget)
	{
		return;
	}

	SettingsWidget->SetShowReturnToMainMenu(true);

	if (!SettingsWidget->IsInViewport())
	{
		SettingsWidget->AddToViewport(100);
	}

	SettingsWidget->CaptureStateForCancel();

	bShowMouseCursor = true;
	FInputModeGameAndUI InputMode;
	InputMode.SetWidgetToFocus(SettingsWidget->TakeWidget());
	InputMode.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);
	InputMode.SetHideCursorDuringCapture(false);
	SetInputMode(InputMode);
	SettingsWidget->SetKeyboardFocus();
	SetPause(true);
}

void AdemoPlayerController::ApplySettingsFromShortcut()
{
	if (SettingsWidget && SettingsWidget->IsInViewport())
	{
		SettingsWidget->ApplyCurrentSettings();
	}
}

void AdemoPlayerController::CloseSettingsMenu()
{
	if (SettingsWidget)
	{
		SettingsWidget->RemoveFromParent();
	}
	HandleSettingsClosed();
}

void AdemoPlayerController::HandleSettingsClosed()
{
	SetPause(false);
	bShowMouseCursor = false;
	SetInputMode(FInputModeGameOnly());
}

void AdemoPlayerController::HandleReturnToMainMenu()
{
	ReturnToMainMenu();
}

void AdemoPlayerController::ReturnToMainMenu()
{
	SetPause(false);
	bShowMouseCursor = false;
	SetInputMode(FInputModeGameOnly());

	if (SettingsWidget)
	{
		SettingsWidget->RemoveFromParent();
	}

	UGameplayStatics::OpenLevel(this, FName(*MainMenuLevelPath), true, MainMenuGameModeOption);
}
