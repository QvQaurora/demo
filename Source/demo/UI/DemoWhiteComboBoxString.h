// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Components/ComboBoxString.h"
#include "DemoWhiteComboBoxString.generated.h"

/** Combo box that renders its labels in white. */
UCLASS()
class UDemoWhiteComboBoxString : public UComboBoxString
{
	GENERATED_BODY()

public:
	UDemoWhiteComboBoxString(const FObjectInitializer& ObjectInitializer);
};
