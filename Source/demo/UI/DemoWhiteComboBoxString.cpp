// Copyright Epic Games, Inc. All Rights Reserved.

#include "DemoWhiteComboBoxString.h"
#include "Styling/CoreStyle.h"

UDemoWhiteComboBoxString::UDemoWhiteComboBoxString(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	InitForegroundColor(FSlateColor(FLinearColor::White));
	InitFont(FCoreStyle::GetDefaultFontStyle(TEXT("Regular"), 16));

	FTableRowStyle RowStyle = GetItemStyle();
	RowStyle.TextColor = FSlateColor(FLinearColor::White);
	RowStyle.SelectedTextColor = FSlateColor(FLinearColor::White);
	SetItemStyle(RowStyle);
}
