// Copyright Epic Games, Inc. All Rights Reserved.

#include "DemoMainMenuWidget.h"

#include "Blueprint/WidgetTree.h"
#include "Components/Border.h"
#include "Components/Button.h"
#include "Components/CanvasPanel.h"
#include "Components/CanvasPanelSlot.h"
#include "Components/SizeBox.h"
#include "Components/TextBlock.h"
#include "Components/VerticalBox.h"
#include "Components/VerticalBoxSlot.h"
#include "Styling/CoreStyle.h"

namespace
{
	const FLinearColor ColorBg(0.03f, 0.05f, 0.07f, 1.f);
	const FLinearColor ColorPanel(0.07f, 0.09f, 0.11f, 0.96f);
	const FLinearColor ColorPanelEdge(0.18f, 0.42f, 0.48f, 1.f);
	const FLinearColor ColorText(0.92f, 0.94f, 0.95f, 1.f);
	const FLinearColor ColorMuted(0.62f, 0.68f, 0.70f, 1.f);
	const FLinearColor ColorButtonPrimary(0.14f, 0.48f, 0.52f, 1.f);
	const FLinearColor ColorButtonSecondary(0.16f, 0.18f, 0.21f, 1.f);
	const FLinearColor ColorButtonQuit(0.42f, 0.16f, 0.16f, 1.f);
	constexpr float PanelWidth = 420.f;
	constexpr float ButtonWidth = 280.f;
	constexpr float ButtonHeight = 52.f;
}

UDemoMainMenuWidget::UDemoMainMenuWidget(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	SetIsFocusable(true);
}

TSharedRef<SWidget> UDemoMainMenuWidget::RebuildWidget()
{
	BuildLayout();
	return Super::RebuildWidget();
}

void UDemoMainMenuWidget::NativeConstruct()
{
	Super::NativeConstruct();
	BuildLayout();
}

void UDemoMainMenuWidget::BuildLayout()
{
	if (bLayoutBuilt || WidgetTree == nullptr)
	{
		return;
	}

	UCanvasPanel* Root = WidgetTree->ConstructWidget<UCanvasPanel>(UCanvasPanel::StaticClass(), TEXT("Root"));
	WidgetTree->RootWidget = Root;

	UBorder* Bg = WidgetTree->ConstructWidget<UBorder>(UBorder::StaticClass(), TEXT("Bg"));
	Bg->SetBrushColor(ColorBg);
	if (UCanvasPanelSlot* BgSlot = Root->AddChildToCanvas(Bg))
	{
		BgSlot->SetAnchors(FAnchors(0.f, 0.f, 1.f, 1.f));
		BgSlot->SetOffsets(FMargin(0.f));
	}

	USizeBox* PanelSize = WidgetTree->ConstructWidget<USizeBox>(USizeBox::StaticClass(), TEXT("PanelSize"));
	PanelSize->SetWidthOverride(PanelWidth);
	if (UCanvasPanelSlot* PanelSlot = Root->AddChildToCanvas(PanelSize))
	{
		PanelSlot->SetAnchors(FAnchors(0.5f, 0.5f));
		PanelSlot->SetAlignment(FVector2D(0.5f, 0.5f));
		PanelSlot->SetAutoSize(true);
	}

	UBorder* Edge = WidgetTree->ConstructWidget<UBorder>(UBorder::StaticClass(), TEXT("Edge"));
	Edge->SetBrushColor(ColorPanelEdge);
	Edge->SetPadding(FMargin(2.f));
	PanelSize->SetContent(Edge);

	UBorder* Panel = WidgetTree->ConstructWidget<UBorder>(UBorder::StaticClass(), TEXT("Panel"));
	Panel->SetBrushColor(ColorPanel);
	Panel->SetPadding(FMargin(36.f, 40.f));
	Edge->SetContent(Panel);

	UVerticalBox* Content = WidgetTree->ConstructWidget<UVerticalBox>(UVerticalBox::StaticClass(), TEXT("Content"));
	Panel->SetContent(Content);

	UTextBlock* Title = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass(), TEXT("Title"));
	Title->SetText(FText::FromString(TEXT("DEMO")));
	Title->SetJustification(ETextJustify::Center);
	Title->SetColorAndOpacity(FSlateColor(ColorText));
	FSlateFontInfo TitleFont = FCoreStyle::GetDefaultFontStyle("Bold", 42);
	Title->SetFont(TitleFont);
	if (UVerticalBoxSlot* TitleSlot = Content->AddChildToVerticalBox(Title))
	{
		TitleSlot->SetHorizontalAlignment(HAlign_Fill);
		TitleSlot->SetPadding(FMargin(0.f, 0.f, 0.f, 8.f));
	}

	UTextBlock* Subtitle = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass(), TEXT("Subtitle"));
	Subtitle->SetText(FText::FromString(TEXT("第三人称演示")));
	Subtitle->SetJustification(ETextJustify::Center);
	Subtitle->SetColorAndOpacity(FSlateColor(ColorMuted));
	Subtitle->SetFont(FCoreStyle::GetDefaultFontStyle("Regular", 16));
	if (UVerticalBoxSlot* SubSlot = Content->AddChildToVerticalBox(Subtitle))
	{
		SubSlot->SetHorizontalAlignment(HAlign_Fill);
		SubSlot->SetPadding(FMargin(0.f, 0.f, 0.f, 32.f));
	}

	auto MakeMenuButton = [this](const FName& Name, const FText& Label, const FLinearColor& Tint) -> UButton*
	{
		UButton* Button = WidgetTree->ConstructWidget<UButton>(UButton::StaticClass(), Name);
		FButtonStyle Style = Button->GetStyle();
		Style.Normal.TintColor = FSlateColor(Tint);
		Style.Hovered.TintColor = FSlateColor(Tint * 1.15f);
		Style.Pressed.TintColor = FSlateColor(Tint * 0.85f);
		Button->SetStyle(Style);

		UTextBlock* LabelText = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass(), *FString::Printf(TEXT("%s_Label"), *Name.ToString()));
		LabelText->SetText(Label);
		LabelText->SetJustification(ETextJustify::Center);
		LabelText->SetColorAndOpacity(FSlateColor(ColorText));
		LabelText->SetFont(FCoreStyle::GetDefaultFontStyle("Bold", 18));
		Button->AddChild(LabelText);
		return Button;
	};

	USizeBox* StartSize = WidgetTree->ConstructWidget<USizeBox>(USizeBox::StaticClass(), TEXT("StartSize"));
	StartSize->SetWidthOverride(ButtonWidth);
	StartSize->SetHeightOverride(ButtonHeight);
	StartButton = MakeMenuButton(TEXT("StartButton"), FText::FromString(TEXT("开始游戏")), ColorButtonPrimary);
	StartButton->OnClicked.AddDynamic(this, &UDemoMainMenuWidget::HandleStartClicked);
	StartSize->SetContent(StartButton);
	if (UVerticalBoxSlot* StartSlot = Content->AddChildToVerticalBox(StartSize))
	{
		StartSlot->SetHorizontalAlignment(HAlign_Center);
		StartSlot->SetPadding(FMargin(0.f, 0.f, 0.f, 14.f));
	}

	USizeBox* SettingsSize = WidgetTree->ConstructWidget<USizeBox>(USizeBox::StaticClass(), TEXT("SettingsSize"));
	SettingsSize->SetWidthOverride(ButtonWidth);
	SettingsSize->SetHeightOverride(ButtonHeight);
	SettingsButton = MakeMenuButton(TEXT("SettingsButton"), FText::FromString(TEXT("设置")), ColorButtonSecondary);
	SettingsButton->OnClicked.AddDynamic(this, &UDemoMainMenuWidget::HandleSettingsClicked);
	SettingsSize->SetContent(SettingsButton);
	if (UVerticalBoxSlot* SettingsSlot = Content->AddChildToVerticalBox(SettingsSize))
	{
		SettingsSlot->SetHorizontalAlignment(HAlign_Center);
		SettingsSlot->SetPadding(FMargin(0.f, 0.f, 0.f, 14.f));
	}

	USizeBox* QuitSize = WidgetTree->ConstructWidget<USizeBox>(USizeBox::StaticClass(), TEXT("QuitSize"));
	QuitSize->SetWidthOverride(ButtonWidth);
	QuitSize->SetHeightOverride(ButtonHeight);
	QuitButton = MakeMenuButton(TEXT("QuitButton"), FText::FromString(TEXT("退出游戏")), ColorButtonQuit);
	QuitButton->OnClicked.AddDynamic(this, &UDemoMainMenuWidget::HandleQuitClicked);
	QuitSize->SetContent(QuitButton);
	if (UVerticalBoxSlot* QuitSlot = Content->AddChildToVerticalBox(QuitSize))
	{
		QuitSlot->SetHorizontalAlignment(HAlign_Center);
	}

	bLayoutBuilt = true;
}

void UDemoMainMenuWidget::HandleStartClicked()
{
	OnStartGame.Broadcast();
}

void UDemoMainMenuWidget::HandleSettingsClicked()
{
	OnOpenSettings.Broadcast();
}

void UDemoMainMenuWidget::HandleQuitClicked()
{
	OnQuitGame.Broadcast();
}
