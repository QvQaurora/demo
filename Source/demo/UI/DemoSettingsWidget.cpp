// Copyright Epic Games, Inc. All Rights Reserved.

#include "DemoSettingsWidget.h"
#include "DemoGameUserSettings.h"
#include "DemoWhiteComboBoxString.h"

#include "Blueprint/WidgetTree.h"
#include "Components/Border.h"
#include "Components/Button.h"
#include "Components/CanvasPanel.h"
#include "Components/CanvasPanelSlot.h"
#include "Components/CheckBox.h"
#include "Components/ComboBoxString.h"
#include "Components/HorizontalBox.h"
#include "Components/HorizontalBoxSlot.h"
#include "Components/SizeBox.h"
#include "Components/ScrollBox.h"
#include "Components/ScrollBoxSlot.h"
#include "Components/Slider.h"
#include "Components/TextBlock.h"
#include "Components/VerticalBox.h"
#include "Components/VerticalBoxSlot.h"
#include "Components/WidgetSwitcher.h"
#include "GameFramework/GameUserSettings.h"
#include "Input/Reply.h"
#include "RHI.h"
#include "Styling/CoreStyle.h"

namespace
{
	const FLinearColor ColorBgDim(0.02f, 0.03f, 0.04f, 0.78f);
	const FLinearColor ColorPanel(0.07f, 0.09f, 0.11f, 0.97f);
	const FLinearColor ColorPanelEdge(0.18f, 0.42f, 0.48f, 1.f);
	const FLinearColor ColorText(0.92f, 0.94f, 0.95f, 1.f);
	const FLinearColor ColorMuted(0.62f, 0.68f, 0.70f, 1.f);
	const FLinearColor ColorAccent(0.22f, 0.72f, 0.78f, 1.f);
	const FLinearColor ColorButtonPrimary(0.14f, 0.48f, 0.52f, 1.f);
	const FLinearColor ColorButtonSecondary(0.16f, 0.18f, 0.21f, 1.f);
	const FLinearColor ColorButtonReturn(0.28f, 0.22f, 0.16f, 1.f);
	const FLinearColor ColorTabIdle(0.12f, 0.14f, 0.16f, 1.f);
	const FLinearColor ColorTabActive(0.16f, 0.42f, 0.46f, 1.f);
	const FLinearColor ColorRow(0.10f, 0.12f, 0.14f, 0.90f);
	constexpr float PanelWidth = 700.f;
	constexpr float LabelWidth = 170.f;
	constexpr float ControlWidth = 340.f;
	constexpr float RowHeight = 46.f;
	// Tall enough for all display/quality rows; ScrollBox covers overflow safely.
	constexpr float PageAreaHeight = 560.f;
	constexpr float ActionButtonWidth = 160.f;
	constexpr float ReturnButtonWidth = 170.f;
}

UDemoSettingsWidget::UDemoSettingsWidget(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	SetIsFocusable(true);
}

TSharedRef<SWidget> UDemoSettingsWidget::RebuildWidget()
{
	BuildLayout();
	return Super::RebuildWidget();
}

void UDemoSettingsWidget::NativeConstruct()
{
	Super::NativeConstruct();
	BuildLayout();
	RefreshFromSettings();
	ShowTab(0);
	SetIsFocusable(true);
}

bool UDemoSettingsWidget::TryHandleEnterKey(const FKeyEvent& InKeyEvent)
{
	if (InKeyEvent.GetKey() == EKeys::Enter)
	{
		ApplyCurrentSettings();
		return true;
	}
	return false;
}

FReply UDemoSettingsWidget::NativeOnPreviewKeyDown(const FGeometry& InGeometry, const FKeyEvent& InKeyEvent)
{
	// Preview catches Enter even when a combo/slider has focus.
	if (TryHandleEnterKey(InKeyEvent))
	{
		return FReply::Handled();
	}
	return Super::NativeOnPreviewKeyDown(InGeometry, InKeyEvent);
}

UTextBlock* UDemoSettingsWidget::MakeText(const FName& Name, const FText& Text, int32 Size, const FLinearColor& Color) const
{
	UTextBlock* Label = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass(), Name);
	Label->SetText(Text);
	Label->SetFont(FCoreStyle::GetDefaultFontStyle("Regular", Size));
	Label->SetColorAndOpacity(FSlateColor(Color));
	Label->SetJustification(ETextJustify::Left);
	return Label;
}

UWidget* UDemoSettingsWidget::MakeSectionTitle(const FName& Name, const FText& Text) const
{
	UHorizontalBox* Row = WidgetTree->ConstructWidget<UHorizontalBox>(UHorizontalBox::StaticClass(), Name);

	UBorder* Bar = WidgetTree->ConstructWidget<UBorder>(UBorder::StaticClass(), *FString::Printf(TEXT("%s_Bar"), *Name.ToString()));
	Bar->SetBrushColor(ColorAccent);
	USizeBox* BarSize = WidgetTree->ConstructWidget<USizeBox>(USizeBox::StaticClass(), *FString::Printf(TEXT("%s_BarSize"), *Name.ToString()));
	BarSize->SetWidthOverride(4.f);
	BarSize->SetHeightOverride(18.f);
	BarSize->SetContent(Bar);
	if (UHorizontalBoxSlot* BarSlot = Row->AddChildToHorizontalBox(BarSize))
	{
		BarSlot->SetVerticalAlignment(VAlign_Center);
		BarSlot->SetPadding(FMargin(0.f, 0.f, 10.f, 0.f));
	}

	UTextBlock* Title = MakeText(*FString::Printf(TEXT("%s_Text"), *Name.ToString()), Text, 16, ColorAccent);
	if (UHorizontalBoxSlot* TitleSlot = Row->AddChildToHorizontalBox(Title))
	{
		TitleSlot->SetVerticalAlignment(VAlign_Center);
	}
	return Row;
}

UWidget* UDemoSettingsWidget::MakeDivider(const FName& Name) const
{
	UBorder* Line = WidgetTree->ConstructWidget<UBorder>(UBorder::StaticClass(), Name);
	Line->SetBrushColor(FLinearColor(1.f, 1.f, 1.f, 0.08f));
	USizeBox* Box = WidgetTree->ConstructWidget<USizeBox>(USizeBox::StaticClass(), *FString::Printf(TEXT("%s_Size"), *Name.ToString()));
	Box->SetHeightOverride(1.f);
	Box->SetContent(Line);
	return Box;
}

UWidget* UDemoSettingsWidget::MakeSettingRow(const FName& Name, const FText& LabelText, UWidget* Control) const
{
	UBorder* RowBorder = WidgetTree->ConstructWidget<UBorder>(UBorder::StaticClass(), *FString::Printf(TEXT("%s_Border"), *Name.ToString()));
	RowBorder->SetBrushColor(ColorRow);
	RowBorder->SetPadding(FMargin(16.f, 8.f));

	UHorizontalBox* Row = WidgetTree->ConstructWidget<UHorizontalBox>(UHorizontalBox::StaticClass(), Name);
	RowBorder->SetContent(Row);

	USizeBox* LabelBox = WidgetTree->ConstructWidget<USizeBox>(USizeBox::StaticClass(), *FString::Printf(TEXT("%s_LabelBox"), *Name.ToString()));
	LabelBox->SetWidthOverride(LabelWidth);
	LabelBox->SetMinDesiredHeight(RowHeight);
	LabelBox->SetContent(MakeText(*FString::Printf(TEXT("%s_Label"), *Name.ToString()), LabelText, 17, ColorText));
	if (UHorizontalBoxSlot* LabelSlot = Row->AddChildToHorizontalBox(LabelBox))
	{
		LabelSlot->SetVerticalAlignment(VAlign_Center);
		LabelSlot->SetPadding(FMargin(0.f, 0.f, 12.f, 0.f));
	}

	USizeBox* ControlBox = WidgetTree->ConstructWidget<USizeBox>(USizeBox::StaticClass(), *FString::Printf(TEXT("%s_ControlBox"), *Name.ToString()));
	ControlBox->SetWidthOverride(ControlWidth);
	ControlBox->SetMinDesiredHeight(RowHeight);
	ControlBox->SetContent(Control);
	if (UHorizontalBoxSlot* ControlSlot = Row->AddChildToHorizontalBox(ControlBox))
	{
		ControlSlot->SetSize(FSlateChildSize(ESlateSizeRule::Fill));
		ControlSlot->SetHorizontalAlignment(HAlign_Fill);
		ControlSlot->SetVerticalAlignment(VAlign_Center);
	}
	return RowBorder;
}

UComboBoxString* UDemoSettingsWidget::MakeCombo(const FName& Name) const
{
	UComboBoxString* Combo = WidgetTree->ConstructWidget<UDemoWhiteComboBoxString>(UDemoWhiteComboBoxString::StaticClass(), Name);
	Combo->SetContentPadding(FMargin(12.f, 8.f));
	return Combo;
}

UButton* UDemoSettingsWidget::MakeActionButton(const FName& Name, const FText& Label, const FLinearColor& Tint) const
{
	UButton* Button = WidgetTree->ConstructWidget<UButton>(UButton::StaticClass(), Name);
	Button->SetBackgroundColor(Tint);

	USizeBox* Size = WidgetTree->ConstructWidget<USizeBox>(USizeBox::StaticClass(), *FString::Printf(TEXT("%s_Size"), *Name.ToString()));
	const float Width = (Name == TEXT("ReturnToMenuButton")) ? ReturnButtonWidth : ActionButtonWidth;
	Size->SetWidthOverride(Width);
	Size->SetHeightOverride(44.f);

	UBorder* Inner = WidgetTree->ConstructWidget<UBorder>(UBorder::StaticClass(), *FString::Printf(TEXT("%s_Inner"), *Name.ToString()));
	Inner->SetBrushColor(FLinearColor(0.f, 0.f, 0.f, 0.f));
	Inner->SetPadding(FMargin(8.f, 0.f));
	Inner->SetHorizontalAlignment(HAlign_Center);
	Inner->SetVerticalAlignment(VAlign_Center);
	UTextBlock* Text = MakeText(*FString::Printf(TEXT("%s_Text"), *Name.ToString()), Label, 17, ColorText);
	Text->SetJustification(ETextJustify::Center);
	Inner->SetContent(Text);
	Size->SetContent(Inner);
	Button->SetContent(Size);
	return Button;
}

UButton* UDemoSettingsWidget::MakeTabButton(const FName& Name, const FText& Label) const
{
	UButton* Button = WidgetTree->ConstructWidget<UButton>(UButton::StaticClass(), Name);
	Button->SetBackgroundColor(ColorTabIdle);

	USizeBox* Size = WidgetTree->ConstructWidget<USizeBox>(USizeBox::StaticClass(), *FString::Printf(TEXT("%s_Size"), *Name.ToString()));
	Size->SetWidthOverride(150.f);
	Size->SetHeightOverride(40.f);

	UTextBlock* Text = MakeText(*FString::Printf(TEXT("%s_Text"), *Name.ToString()), Label, 16, ColorText);
	Text->SetJustification(ETextJustify::Center);
	UBorder* Inner = WidgetTree->ConstructWidget<UBorder>(UBorder::StaticClass(), *FString::Printf(TEXT("%s_Inner"), *Name.ToString()));
	Inner->SetBrushColor(FLinearColor(0.f, 0.f, 0.f, 0.f));
	Inner->SetHorizontalAlignment(HAlign_Center);
	Inner->SetVerticalAlignment(VAlign_Center);
	Inner->SetContent(Text);
	Size->SetContent(Inner);
	Button->SetContent(Size);
	return Button;
}

USlider* UDemoSettingsWidget::MakeVolumeSlider(const FName& Name) const
{
	USlider* Slider = WidgetTree->ConstructWidget<USlider>(USlider::StaticClass(), Name);
	Slider->SetMinValue(0.f);
	Slider->SetMaxValue(1.f);
	Slider->SetStepSize(0.01f);
	Slider->SetValue(1.f);
	return Slider;
}

UVerticalBox* UDemoSettingsWidget::BuildGraphicsPage()
{
	UVerticalBox* Page = WidgetTree->ConstructWidget<UVerticalBox>(UVerticalBox::StaticClass(), TEXT("GraphicsPage"));

	auto AddSection = [&](const FName& SectionName, const FText& TitleText)
	{
		if (UVerticalBoxSlot* SectionSlot = Page->AddChildToVerticalBox(MakeSectionTitle(SectionName, TitleText)))
		{
			SectionSlot->SetPadding(FMargin(0.f, 10.f, 0.f, 10.f));
		}
	};
	auto AddRow = [&](const FName& RowName, const FText& LabelText, UWidget* Control)
	{
		if (UVerticalBoxSlot* RowSlot = Page->AddChildToVerticalBox(MakeSettingRow(RowName, LabelText, Control)))
		{
			RowSlot->SetPadding(FMargin(0.f, 0.f, 0.f, 8.f));
		}
	};

	AddSection(TEXT("SectionDisplay"), FText::FromString(TEXT("显示")));

	WindowModeCombo = MakeCombo(TEXT("WindowModeCombo"));
	WindowModeCombo->AddOption(TEXT("全屏"));
	WindowModeCombo->AddOption(TEXT("无边框窗口"));
	WindowModeCombo->AddOption(TEXT("窗口化"));
	AddRow(TEXT("WindowRow"), FText::FromString(TEXT("显示模式")), WindowModeCombo);

	ResolutionCombo = MakeCombo(TEXT("ResolutionCombo"));
	PopulateResolutions();
	AddRow(TEXT("ResolutionRow"), FText::FromString(TEXT("分辨率")), ResolutionCombo);

	FrameLimitCombo = MakeCombo(TEXT("FrameLimitCombo"));
	FrameLimitCombo->AddOption(TEXT("无限制"));
	FrameLimitCombo->AddOption(TEXT("30 FPS"));
	FrameLimitCombo->AddOption(TEXT("60 FPS"));
	FrameLimitCombo->AddOption(TEXT("120 FPS"));
	FrameLimitCombo->AddOption(TEXT("144 FPS"));
	AddRow(TEXT("FrameRow"), FText::FromString(TEXT("帧率上限")), FrameLimitCombo);

	VSyncCheck = WidgetTree->ConstructWidget<UCheckBox>(UCheckBox::StaticClass(), TEXT("VSyncCheck"));
	AddRow(TEXT("VSyncRow"), FText::FromString(TEXT("垂直同步")), VSyncCheck);

	AddSection(TEXT("SectionGraphics"), FText::FromString(TEXT("画质")));

	QualityCombo = MakeCombo(TEXT("QualityCombo"));
	QualityCombo->AddOption(TEXT("低"));
	QualityCombo->AddOption(TEXT("中"));
	QualityCombo->AddOption(TEXT("高"));
	QualityCombo->AddOption(TEXT("极高"));
	AddRow(TEXT("QualityRow"), FText::FromString(TEXT("画质预设")), QualityCombo);

	ScaleCombo = MakeCombo(TEXT("ScaleCombo"));
	ScaleCombo->AddOption(TEXT("50%  流畅优先"));
	ScaleCombo->AddOption(TEXT("75%  均衡"));
	ScaleCombo->AddOption(TEXT("100% 原生"));
	AddRow(TEXT("ScaleRow"), FText::FromString(TEXT("渲染比例")), ScaleCombo);

	MotionBlurCheck = WidgetTree->ConstructWidget<UCheckBox>(UCheckBox::StaticClass(), TEXT("MotionBlurCheck"));
	AddRow(TEXT("MotionBlurRow"), FText::FromString(TEXT("动态模糊")), MotionBlurCheck);

	return Page;
}

UVerticalBox* UDemoSettingsWidget::BuildAudioPage()
{
	UVerticalBox* Page = WidgetTree->ConstructWidget<UVerticalBox>(UVerticalBox::StaticClass(), TEXT("AudioPage"));

	if (UVerticalBoxSlot* SectionSlot = Page->AddChildToVerticalBox(MakeSectionTitle(TEXT("SectionAudio"), FText::FromString(TEXT("音量")))))
	{
		SectionSlot->SetPadding(FMargin(0.f, 10.f, 0.f, 10.f));
	}

	auto AddVolumeRow = [&](const FName& RowName, const FText& LabelText, TObjectPtr<USlider>& OutSlider, TObjectPtr<UTextBlock>& OutValueText)
	{
		UHorizontalBox* ControlRow = WidgetTree->ConstructWidget<UHorizontalBox>(UHorizontalBox::StaticClass(), *FString::Printf(TEXT("%s_Control"), *RowName.ToString()));
		OutSlider = MakeVolumeSlider(*FString::Printf(TEXT("%s_Slider"), *RowName.ToString()));
		OutValueText = MakeText(*FString::Printf(TEXT("%s_Value"), *RowName.ToString()), FText::FromString(TEXT("100%")), 16, ColorMuted);
		OutValueText->SetJustification(ETextJustify::Right);

		USizeBox* SliderBox = WidgetTree->ConstructWidget<USizeBox>(USizeBox::StaticClass(), *FString::Printf(TEXT("%s_SliderBox"), *RowName.ToString()));
		SliderBox->SetWidthOverride(250.f);
		SliderBox->SetContent(OutSlider);
		if (UHorizontalBoxSlot* SliderSlot = ControlRow->AddChildToHorizontalBox(SliderBox))
		{
			SliderSlot->SetSize(FSlateChildSize(ESlateSizeRule::Fill));
			SliderSlot->SetVerticalAlignment(VAlign_Center);
			SliderSlot->SetPadding(FMargin(0.f, 0.f, 12.f, 0.f));
		}

		USizeBox* ValueBox = WidgetTree->ConstructWidget<USizeBox>(USizeBox::StaticClass(), *FString::Printf(TEXT("%s_ValueBox"), *RowName.ToString()));
		ValueBox->SetWidthOverride(58.f);
		ValueBox->SetContent(OutValueText);
		if (UHorizontalBoxSlot* ValueSlot = ControlRow->AddChildToHorizontalBox(ValueBox))
		{
			ValueSlot->SetVerticalAlignment(VAlign_Center);
		}

		if (UVerticalBoxSlot* RowSlot = Page->AddChildToVerticalBox(MakeSettingRow(RowName, LabelText, ControlRow)))
		{
			RowSlot->SetPadding(FMargin(0.f, 0.f, 0.f, 8.f));
		}
	};

	AddVolumeRow(TEXT("MasterRow"), FText::FromString(TEXT("主音量")), MasterVolumeSlider, MasterVolumeValueText);
	MasterVolumeSlider->OnValueChanged.AddDynamic(this, &UDemoSettingsWidget::HandleMasterVolumeChanged);

	AddVolumeRow(TEXT("MusicRow"), FText::FromString(TEXT("音乐")), MusicVolumeSlider, MusicVolumeValueText);
	MusicVolumeSlider->OnValueChanged.AddDynamic(this, &UDemoSettingsWidget::HandleMusicVolumeChanged);

	AddVolumeRow(TEXT("SFXRow"), FText::FromString(TEXT("音效")), SFXVolumeSlider, SFXVolumeValueText);
	SFXVolumeSlider->OnValueChanged.AddDynamic(this, &UDemoSettingsWidget::HandleSFXVolumeChanged);

	UTextBlock* Tip = MakeText(TEXT("AudioTip"), FText::FromString(TEXT("主音量立即生效；点应用后保存全部声音设置")), 13, ColorMuted);
	if (UVerticalBoxSlot* TipSlot = Page->AddChildToVerticalBox(Tip))
	{
		TipSlot->SetPadding(FMargin(4.f, 14.f, 0.f, 0.f));
	}

	return Page;
}

void UDemoSettingsWidget::BuildLayout()
{
	if (bLayoutBuilt || WidgetTree == nullptr)
	{
		return;
	}

	UCanvasPanel* Root = WidgetTree->ConstructWidget<UCanvasPanel>(UCanvasPanel::StaticClass(), TEXT("Root"));
	WidgetTree->RootWidget = Root;

	UBorder* Dim = WidgetTree->ConstructWidget<UBorder>(UBorder::StaticClass(), TEXT("Dim"));
	Dim->SetBrushColor(ColorBgDim);
	if (UCanvasPanelSlot* DimSlot = Root->AddChildToCanvas(Dim))
	{
		DimSlot->SetAnchors(FAnchors(0.f, 0.f, 1.f, 1.f));
		DimSlot->SetOffsets(FMargin(0.f));
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
	Panel->SetPadding(FMargin(28.f, 26.f));
	Edge->SetContent(Panel);

	UVerticalBox* Content = WidgetTree->ConstructWidget<UVerticalBox>(UVerticalBox::StaticClass(), TEXT("Content"));
	Panel->SetContent(Content);

	UTextBlock* Title = MakeText(TEXT("Title"), FText::FromString(TEXT("游戏设置")), 30, ColorText);
	Title->SetJustification(ETextJustify::Center);
	if (UVerticalBoxSlot* TitleSlot = Content->AddChildToVerticalBox(Title))
	{
		TitleSlot->SetHorizontalAlignment(HAlign_Fill);
		TitleSlot->SetPadding(FMargin(0.f, 0.f, 0.f, 4.f));
	}

	UTextBlock* Subtitle = MakeText(TEXT("Subtitle"), FText::FromString(TEXT("显示 / 画质 / 声音")), 14, ColorMuted);
	Subtitle->SetJustification(ETextJustify::Center);
	if (UVerticalBoxSlot* SubSlot = Content->AddChildToVerticalBox(Subtitle))
	{
		SubSlot->SetHorizontalAlignment(HAlign_Fill);
		SubSlot->SetPadding(FMargin(0.f, 0.f, 0.f, 14.f));
	}

	UHorizontalBox* Tabs = WidgetTree->ConstructWidget<UHorizontalBox>(UHorizontalBox::StaticClass(), TEXT("Tabs"));
	if (UVerticalBoxSlot* TabsSlot = Content->AddChildToVerticalBox(Tabs))
	{
		TabsSlot->SetHorizontalAlignment(HAlign_Center);
		TabsSlot->SetPadding(FMargin(0.f, 0.f, 0.f, 14.f));
	}

	GraphicsTabButton = MakeTabButton(TEXT("GraphicsTab"), FText::FromString(TEXT("显示与画质")));
	GraphicsTabButton->OnClicked.AddDynamic(this, &UDemoSettingsWidget::HandleGraphicsTabClicked);
	if (UHorizontalBoxSlot* GTabSlot = Tabs->AddChildToHorizontalBox(GraphicsTabButton))
	{
		GTabSlot->SetPadding(FMargin(0.f, 0.f, 10.f, 0.f));
	}

	AudioTabButton = MakeTabButton(TEXT("AudioTab"), FText::FromString(TEXT("声音")));
	AudioTabButton->OnClicked.AddDynamic(this, &UDemoSettingsWidget::HandleAudioTabClicked);
	Tabs->AddChildToHorizontalBox(AudioTabButton);

	Content->AddChildToVerticalBox(MakeDivider(TEXT("DivTop")));

	USizeBox* PageArea = WidgetTree->ConstructWidget<USizeBox>(USizeBox::StaticClass(), TEXT("PageArea"));
	PageArea->SetHeightOverride(PageAreaHeight);
	if (UVerticalBoxSlot* PageAreaSlot = Content->AddChildToVerticalBox(PageArea))
	{
		PageAreaSlot->SetPadding(FMargin(0.f, 8.f, 0.f, 0.f));
		PageAreaSlot->SetHorizontalAlignment(HAlign_Fill);
	}

	PageSwitcher = WidgetTree->ConstructWidget<UWidgetSwitcher>(UWidgetSwitcher::StaticClass(), TEXT("PageSwitcher"));
	PageArea->SetContent(PageSwitcher);

	auto WrapPage = [this](UWidget* Page, const FName& ScrollName) -> UWidget*
	{
		UScrollBox* Scroll = WidgetTree->ConstructWidget<UScrollBox>(UScrollBox::StaticClass(), ScrollName);
		Scroll->SetAnimateWheelScrolling(true);
		Scroll->SetScrollBarVisibility(ESlateVisibility::Visible);
		if (UScrollBoxSlot* PageSlot = Cast<UScrollBoxSlot>(Scroll->AddChild(Page)))
		{
			PageSlot->SetHorizontalAlignment(HAlign_Fill);
			PageSlot->SetPadding(FMargin(0.f, 0.f, 8.f, 0.f));
		}
		return Scroll;
	};

	PageSwitcher->AddChild(WrapPage(BuildGraphicsPage(), TEXT("GraphicsScroll")));
	PageSwitcher->AddChild(WrapPage(BuildAudioPage(), TEXT("AudioScroll")));

	if (UVerticalBoxSlot* DivSlot = Content->AddChildToVerticalBox(MakeDivider(TEXT("DivBottom"))))
	{
		DivSlot->SetPadding(FMargin(0.f, 12.f, 0.f, 0.f));
	}

	UHorizontalBox* Buttons = WidgetTree->ConstructWidget<UHorizontalBox>(UHorizontalBox::StaticClass(), TEXT("Buttons"));
	if (UVerticalBoxSlot* ButtonsSlot = Content->AddChildToVerticalBox(Buttons))
	{
		ButtonsSlot->SetPadding(FMargin(0.f, 18.f, 0.f, 0.f));
		ButtonsSlot->SetHorizontalAlignment(HAlign_Center);
	}

	ApplyButton = MakeActionButton(TEXT("ApplyButton"), FText::FromString(TEXT("应用")), ColorButtonPrimary);
	ApplyButton->OnClicked.AddDynamic(this, &UDemoSettingsWidget::HandleApplyClicked);
	if (UHorizontalBoxSlot* ApplySlot = Buttons->AddChildToHorizontalBox(ApplyButton))
	{
		ApplySlot->SetPadding(FMargin(0.f, 0.f, 12.f, 0.f));
	}

	CloseButton = MakeActionButton(TEXT("CancelButton"), FText::FromString(TEXT("取消")), ColorButtonSecondary);
	CloseButton->OnClicked.AddDynamic(this, &UDemoSettingsWidget::HandleCancelClicked);
	if (UHorizontalBoxSlot* CloseSlot = Buttons->AddChildToHorizontalBox(CloseButton))
	{
		CloseSlot->SetPadding(FMargin(0.f, 0.f, 12.f, 0.f));
	}

	ReturnToMenuButton = MakeActionButton(TEXT("ReturnToMenuButton"), FText::FromString(TEXT("返回主菜单")), ColorButtonReturn);
	ReturnToMenuButton->OnClicked.AddDynamic(this, &UDemoSettingsWidget::HandleReturnToMainMenuClicked);
	Buttons->AddChildToHorizontalBox(ReturnToMenuButton);
	ReturnToMenuButton->SetVisibility(bShowReturnToMainMenu ? ESlateVisibility::Visible : ESlateVisibility::Collapsed);

	UTextBlock* Hint = MakeText(TEXT("Hint"), FText::FromString(TEXT("回车 = 应用并关闭  ·  编辑器按 P/O 打开")), 13, ColorMuted);
	Hint->SetJustification(ETextJustify::Center);
	if (UVerticalBoxSlot* HintSlot = Content->AddChildToVerticalBox(Hint))
	{
		HintSlot->SetPadding(FMargin(0.f, 16.f, 0.f, 0.f));
		HintSlot->SetHorizontalAlignment(HAlign_Fill);
	}

	bLayoutBuilt = true;
}

void UDemoSettingsWidget::ShowTab(int32 TabIndex)
{
	ActiveTabIndex = TabIndex;
	if (PageSwitcher)
	{
		PageSwitcher->SetActiveWidgetIndex(TabIndex);
	}
	if (GraphicsTabButton)
	{
		GraphicsTabButton->SetBackgroundColor(TabIndex == 0 ? ColorTabActive : ColorTabIdle);
	}
	if (AudioTabButton)
	{
		AudioTabButton->SetBackgroundColor(TabIndex == 1 ? ColorTabActive : ColorTabIdle);
	}
}

void UDemoSettingsWidget::HandleGraphicsTabClicked()
{
	ShowTab(0);
}

void UDemoSettingsWidget::HandleAudioTabClicked()
{
	ShowTab(1);
}

void UDemoSettingsWidget::UpdateVolumeLabels()
{
	auto SetPct = [](UTextBlock* Text, float Value)
	{
		if (Text)
		{
			Text->SetText(FText::FromString(FString::Printf(TEXT("%d%%"), FMath::RoundToInt(Value * 100.f))));
		}
	};
	SetPct(MasterVolumeValueText, MasterVolumeSlider ? MasterVolumeSlider->GetValue() : 1.f);
	SetPct(MusicVolumeValueText, MusicVolumeSlider ? MusicVolumeSlider->GetValue() : 1.f);
	SetPct(SFXVolumeValueText, SFXVolumeSlider ? SFXVolumeSlider->GetValue() : 1.f);
}

void UDemoSettingsWidget::HandleMasterVolumeChanged(float Value)
{
	UpdateVolumeLabels();
	if (UDemoGameUserSettings* Settings = UDemoGameUserSettings::GetDemoGameUserSettings())
	{
		Settings->SetMasterVolume(Value);
		Settings->ApplyAudioSettings();
	}
}

void UDemoSettingsWidget::HandleMusicVolumeChanged(float Value)
{
	UpdateVolumeLabels();
}

void UDemoSettingsWidget::HandleSFXVolumeChanged(float Value)
{
	UpdateVolumeLabels();
}

void UDemoSettingsWidget::PopulateResolutions()
{
	if (!ResolutionCombo)
	{
		return;
	}

	ResolutionCombo->ClearOptions();
	FScreenResolutionArray Resolutions;
	TSet<FString> Unique;
	if (RHIGetAvailableResolutions(Resolutions, true))
	{
		for (const FScreenResolutionRHI& Res : Resolutions)
		{
			if (Res.Width < 1280 || Res.Height < 720)
			{
				continue;
			}
			const FString Option = FString::Printf(TEXT("%d × %d"), Res.Width, Res.Height);
			if (!Unique.Contains(Option))
			{
				Unique.Add(Option);
				ResolutionCombo->AddOption(Option);
			}
		}
	}
	if (ResolutionCombo->GetOptionCount() == 0)
	{
		ResolutionCombo->AddOption(TEXT("1280 × 720"));
		ResolutionCombo->AddOption(TEXT("1600 × 900"));
		ResolutionCombo->AddOption(TEXT("1920 × 1080"));
		ResolutionCombo->AddOption(TEXT("2560 × 1440"));
	}
}

void UDemoSettingsWidget::RefreshFromSettings()
{
	UDemoGameUserSettings* Settings = UDemoGameUserSettings::GetDemoGameUserSettings();
	if (!Settings)
	{
		return;
	}

	if (QualityCombo)
	{
		// GetOverallScalabilityLevel() returns -1 when resolution scale is customized (e.g. 100%),
		// which previously clamped to 0 and looked like "Low".
		const int32 Level = Settings->GetPreferredOverallQuality();
		QualityCombo->SetSelectedIndex(FMath::Clamp(Level, 0, 3));
	}

	float CurrentScale = 100.f;
	float DummyMin = 0.f;
	float DummyMax = 100.f;
	Settings->GetResolutionScaleInformationEx(DummyMin, CurrentScale, DummyMax, DummyMax);

	if (WindowModeCombo)
	{
		switch (Settings->GetFullscreenMode())
		{
		case EWindowMode::Fullscreen: WindowModeCombo->SetSelectedIndex(0); break;
		case EWindowMode::WindowedFullscreen: WindowModeCombo->SetSelectedIndex(1); break;
		default: WindowModeCombo->SetSelectedIndex(2); break;
		}
	}

	if (ResolutionCombo)
	{
		const FIntPoint Res = Settings->GetScreenResolution();
		const FString Option = FString::Printf(TEXT("%d × %d"), Res.X, Res.Y);
		if (ResolutionCombo->FindOptionIndex(Option) == INDEX_NONE)
		{
			ResolutionCombo->AddOption(Option);
		}
		ResolutionCombo->SetSelectedOption(Option);
	}

	if (ScaleCombo)
	{
		if (CurrentScale <= 55.f) ScaleCombo->SetSelectedIndex(0);
		else if (CurrentScale <= 85.f) ScaleCombo->SetSelectedIndex(1);
		else ScaleCombo->SetSelectedIndex(2);
	}

	if (FrameLimitCombo)
	{
		const float Limit = Settings->GetFrameRateLimit();
		if (Limit <= 0.f) FrameLimitCombo->SetSelectedIndex(0);
		else if (Limit <= 30.f) FrameLimitCombo->SetSelectedIndex(1);
		else if (Limit <= 60.f) FrameLimitCombo->SetSelectedIndex(2);
		else if (Limit <= 120.f) FrameLimitCombo->SetSelectedIndex(3);
		else FrameLimitCombo->SetSelectedIndex(4);
	}

	if (VSyncCheck) VSyncCheck->SetIsChecked(Settings->IsVSyncEnabled());
	if (MotionBlurCheck) MotionBlurCheck->SetIsChecked(Settings->IsMotionBlurEnabled());

	if (MasterVolumeSlider) MasterVolumeSlider->SetValue(Settings->GetMasterVolume());
	if (MusicVolumeSlider) MusicVolumeSlider->SetValue(Settings->GetMusicVolume());
	if (SFXVolumeSlider) SFXVolumeSlider->SetValue(Settings->GetSFXVolume());
	UpdateVolumeLabels();
}

void UDemoSettingsWidget::CaptureStateForCancel()
{
	if (UDemoGameUserSettings* Settings = UDemoGameUserSettings::GetDemoGameUserSettings())
	{
		CapturedMasterVolume = Settings->GetMasterVolume();
		CapturedMusicVolume = Settings->GetMusicVolume();
		CapturedSFXVolume = Settings->GetSFXVolume();
		bCapturedMotionBlur = Settings->IsMotionBlurEnabled();
		bHasCapturedState = true;
	}
	RefreshFromSettings();
}

void UDemoSettingsWidget::CancelAndClose()
{
	if (bHasCapturedState)
	{
		if (UDemoGameUserSettings* Settings = UDemoGameUserSettings::GetDemoGameUserSettings())
		{
			Settings->SetMasterVolume(CapturedMasterVolume);
			Settings->SetMusicVolume(CapturedMusicVolume);
			Settings->SetSFXVolume(CapturedSFXVolume);
			Settings->SetMotionBlurEnabled(bCapturedMotionBlur);
			Settings->ApplyAudioSettings();
			Settings->ApplyExtraGraphicsSettings();
		}
	}
	OnClosed.Broadcast();
	RemoveFromParent();
}

void UDemoSettingsWidget::ApplyCurrentSettings()
{
	HandleApplyClicked();
}

void UDemoSettingsWidget::HandleApplyClicked()
{
	UDemoGameUserSettings* Settings = UDemoGameUserSettings::GetDemoGameUserSettings();
	if (!Settings)
	{
		return;
	}

	if (QualityCombo && QualityCombo->GetSelectedIndex() != INDEX_NONE)
	{
		Settings->SetPreferredOverallQuality(QualityCombo->GetSelectedIndex());
		Settings->SetOverallScalabilityLevel(QualityCombo->GetSelectedIndex());
	}

	EWindowMode::Type DesiredMode = EWindowMode::Windowed;
	if (WindowModeCombo)
	{
		switch (WindowModeCombo->GetSelectedIndex())
		{
		case 0: DesiredMode = EWindowMode::Fullscreen; break;
		case 1: DesiredMode = EWindowMode::WindowedFullscreen; break;
		default: DesiredMode = EWindowMode::Windowed; break;
		}
	}

	FIntPoint DesiredRes = Settings->GetScreenResolution();
	if (ResolutionCombo)
	{
		FString Selected = ResolutionCombo->GetSelectedOption();
		Selected.ReplaceInline(TEXT(" "), TEXT(""));
		Selected.ReplaceInline(TEXT("×"), TEXT("x"));
		FString WidthStr, HeightStr;
		if (Selected.Split(TEXT("x"), &WidthStr, &HeightStr))
		{
			DesiredRes = FIntPoint(FCString::Atoi(*WidthStr), FCString::Atoi(*HeightStr));
		}
	}

	const FIntPoint AppliedRes = Settings->ApplyDisplayMode(DesiredMode, DesiredRes);

	if (ScaleCombo)
	{
		switch (ScaleCombo->GetSelectedIndex())
		{
		case 0: Settings->SetResolutionScaleValueEx(50.f); break;
		case 1: Settings->SetResolutionScaleValueEx(75.f); break;
		default: Settings->SetResolutionScaleValueEx(100.f); break;
		}
	}

	if (FrameLimitCombo)
	{
		switch (FrameLimitCombo->GetSelectedIndex())
		{
		case 1: Settings->SetFrameRateLimit(30.f); break;
		case 2: Settings->SetFrameRateLimit(60.f); break;
		case 3: Settings->SetFrameRateLimit(120.f); break;
		case 4: Settings->SetFrameRateLimit(144.f); break;
		default: Settings->SetFrameRateLimit(0.f); break;
		}
	}

	if (VSyncCheck) Settings->SetVSyncEnabled(VSyncCheck->IsChecked());
	if (MotionBlurCheck) Settings->SetMotionBlurEnabled(MotionBlurCheck->IsChecked());

	if (MasterVolumeSlider) Settings->SetMasterVolume(MasterVolumeSlider->GetValue());
	if (MusicVolumeSlider) Settings->SetMusicVolume(MusicVolumeSlider->GetValue());
	if (SFXVolumeSlider) Settings->SetSFXVolume(SFXVolumeSlider->GetValue());

	// Non-resolution settings (quality/vsync/audio/etc). Display already applied above.
	Settings->ApplyNonResolutionSettings();
	Settings->SaveSettings();

	if (ResolutionCombo && AppliedRes != DesiredRes)
	{
		PopulateResolutions();
		const FString Option = FString::Printf(TEXT("%d × %d"), AppliedRes.X, AppliedRes.Y);
		if (ResolutionCombo->FindOptionIndex(Option) == INDEX_NONE)
		{
			ResolutionCombo->AddOption(Option);
		}
		ResolutionCombo->SetSelectedOption(Option);
	}

	bHasCapturedState = false;
	OnClosed.Broadcast();
	RemoveFromParent();
}

void UDemoSettingsWidget::HandleCancelClicked()
{
	CancelAndClose();
}

void UDemoSettingsWidget::SetShowReturnToMainMenu(bool bShow)
{
	bShowReturnToMainMenu = bShow;
	if (ReturnToMenuButton)
	{
		ReturnToMenuButton->SetVisibility(bShow ? ESlateVisibility::Visible : ESlateVisibility::Collapsed);
	}
}

void UDemoSettingsWidget::HandleReturnToMainMenuClicked()
{
	bHasCapturedState = false;
	OnReturnToMainMenu.Broadcast();
	RemoveFromParent();
}
