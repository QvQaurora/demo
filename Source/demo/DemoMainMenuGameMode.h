// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "DemoMainMenuGameMode.generated.h"

/**
 * Lightweight GameMode for the title screen. No gameplay pawn.
 */
UCLASS()
class ADemoMainMenuGameMode : public AGameModeBase
{
	GENERATED_BODY()

public:
	ADemoMainMenuGameMode();
};
