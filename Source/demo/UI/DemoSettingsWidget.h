// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "DemoSettingsWidget.generated.h"

class UComboBoxString;
class UCheckBox;
class UButton;
class UTextBlock;
class USlider;
class UWidgetSwitcher;
class UVerticalBox;
class UWidgetTree;

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnDemoSettingsClosed);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnDemoSettingsReturnToMainMenu);

/**
 * In-game settings / pause menu with Graphics + Audio tabs.
 */
UCLASS()
class UDemoSettingsWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	UPROPERTY(BlueprintAssignable, Category = "Settings")
	FOnDemoSettingsClosed OnClosed;

	UPROPERTY(BlueprintAssignable, Category = "Settings")
	FOnDemoSettingsReturnToMainMenu OnReturnToMainMenu;

	UDemoSettingsWidget(const FObjectInitializer& ObjectInitializer);

	/** Apply current UI values (also used by Enter shortcut) and close. */
	void ApplyCurrentSettings();

	/** Remember settings when the menu opens so Cancel can restore them. */
	void CaptureStateForCancel();

	/** Discard edits and close. */
	void CancelAndClose();

	/** Show/hide the in-game "Return to Main Menu" action. */
	void SetShowReturnToMainMenu(bool bShow);

protected:
	virtual TSharedRef<SWidget> RebuildWidget() override;
	virtual void NativeConstruct() override;
	virtual FReply NativeOnPreviewKeyDown(const FGeometry& InGeometry, const FKeyEvent& InKeyEvent) override;

	void BuildLayout();
	void RefreshFromSettings();
	void PopulateResolutions();
	void ShowTab(int32 TabIndex);
	void UpdateVolumeLabels();

	UTextBlock* MakeText(const FName& Name, const FText& Text, int32 Size, const FLinearColor& Color) const;
	UWidget* MakeSectionTitle(const FName& Name, const FText& Text) const;
	UWidget* MakeDivider(const FName& Name) const;
	UWidget* MakeSettingRow(const FName& Name, const FText& LabelText, UWidget* Control) const;
	UComboBoxString* MakeCombo(const FName& Name) const;
	UButton* MakeActionButton(const FName& Name, const FText& Label, const FLinearColor& Tint) const;
	UButton* MakeTabButton(const FName& Name, const FText& Label) const;
	USlider* MakeVolumeSlider(const FName& Name) const;
	UVerticalBox* BuildGraphicsPage();
	UVerticalBox* BuildAudioPage();

	UFUNCTION()
	void HandleApplyClicked();

	UFUNCTION()
	void HandleCancelClicked();

	UFUNCTION()
	void HandleReturnToMainMenuClicked();

	bool TryHandleEnterKey(const FKeyEvent& InKeyEvent);

	float CapturedMasterVolume = 1.f;
	float CapturedMusicVolume = 1.f;
	float CapturedSFXVolume = 1.f;
	bool bCapturedMotionBlur = false;
	bool bHasCapturedState = false;

	UFUNCTION()
	void HandleGraphicsTabClicked();

	UFUNCTION()
	void HandleAudioTabClicked();

	UFUNCTION()
	void HandleMasterVolumeChanged(float Value);

	UFUNCTION()
	void HandleMusicVolumeChanged(float Value);

	UFUNCTION()
	void HandleSFXVolumeChanged(float Value);

	UPROPERTY()
	TObjectPtr<UWidgetSwitcher> PageSwitcher = nullptr;

	UPROPERTY()
	TObjectPtr<UButton> GraphicsTabButton = nullptr;

	UPROPERTY()
	TObjectPtr<UButton> AudioTabButton = nullptr;

	UPROPERTY()
	TObjectPtr<UComboBoxString> QualityCombo = nullptr;

	UPROPERTY()
	TObjectPtr<UComboBoxString> WindowModeCombo = nullptr;

	UPROPERTY()
	TObjectPtr<UComboBoxString> ResolutionCombo = nullptr;

	UPROPERTY()
	TObjectPtr<UComboBoxString> FrameLimitCombo = nullptr;

	UPROPERTY()
	TObjectPtr<UComboBoxString> ScaleCombo = nullptr;

	UPROPERTY()
	TObjectPtr<UCheckBox> VSyncCheck = nullptr;

	UPROPERTY()
	TObjectPtr<UCheckBox> MotionBlurCheck = nullptr;

	UPROPERTY()
	TObjectPtr<USlider> MasterVolumeSlider = nullptr;

	UPROPERTY()
	TObjectPtr<USlider> MusicVolumeSlider = nullptr;

	UPROPERTY()
	TObjectPtr<USlider> SFXVolumeSlider = nullptr;

	UPROPERTY()
	TObjectPtr<UTextBlock> MasterVolumeValueText = nullptr;

	UPROPERTY()
	TObjectPtr<UTextBlock> MusicVolumeValueText = nullptr;

	UPROPERTY()
	TObjectPtr<UTextBlock> SFXVolumeValueText = nullptr;

	UPROPERTY()
	TObjectPtr<UButton> ApplyButton = nullptr;

	UPROPERTY()
	TObjectPtr<UButton> CloseButton = nullptr;

	UPROPERTY()
	TObjectPtr<UButton> ReturnToMenuButton = nullptr;

	int32 ActiveTabIndex = 0;
	bool bLayoutBuilt = false;
	bool bShowReturnToMainMenu = false;
};
