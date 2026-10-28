// Copyright Epic Games, Inc. All Rights Reserved.

#include "DemoMainMenuGameMode.h"
#include "DemoMainMenuPlayerController.h"
#include "GameFramework/SpectatorPawn.h"

ADemoMainMenuGameMode::ADemoMainMenuGameMode()
{
	PlayerControllerClass = ADemoMainMenuPlayerController::StaticClass();
	DefaultPawnClass = ASpectatorPawn::StaticClass();
	bStartPlayersAsSpectators = true;
	HUDClass = nullptr;
}
