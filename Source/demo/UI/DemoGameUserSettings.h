// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameUserSettings.h"
#include "DemoGameUserSettings.generated.h"

/**
 * Extended game user settings with graphics + audio options.
 */
UCLASS(config = GameUserSettings)
class UDemoGameUserSettings : public UGameUserSettings
{
	GENERATED_BODY()

public:
	UDemoGameUserSettings();

	UFUNCTION(BlueprintCallable, Category = "Settings")
	static UDemoGameUserSettings* GetDemoGameUserSettings();

	virtual void SetToDefaults() override;
	virtual void LoadSettings(bool bForceReload = false) override;
	virtual void ApplySettings(bool bCheckForCommandLineOverrides) override;
	virtual void ApplyNonResolutionSettings() override;

	UFUNCTION(BlueprintCallable, Category = "Settings")
	void SetMotionBlurEnabled(bool bEnabled);

	UFUNCTION(BlueprintPure, Category = "Settings")
	bool IsMotionBlurEnabled() const { return bMotionBlurEnabled; }

	UFUNCTION(BlueprintCallable, Category = "Settings")
	void SetMasterVolume(float Volume);

	UFUNCTION(BlueprintPure, Category = "Settings")
	float GetMasterVolume() const { return MasterVolume; }

	UFUNCTION(BlueprintCallable, Category = "Settings")
	void SetMusicVolume(float Volume);

	UFUNCTION(BlueprintPure, Category = "Settings")
	float GetMusicVolume() const { return MusicVolume; }

	UFUNCTION(BlueprintCallable, Category = "Settings")
	void SetSFXVolume(float Volume);

	UFUNCTION(BlueprintPure, Category = "Settings")
	float GetSFXVolume() const { return SFXVolume; }

	UFUNCTION(BlueprintCallable, Category = "Settings")
	void SetPreferredOverallQuality(int32 Level);

	UFUNCTION(BlueprintPure, Category = "Settings")
	int32 GetPreferredOverallQuality() const { return PreferredOverallQuality; }

	/** Windowed + High quality + 100% scale. */
	void ApplyPreferredDefaults();

	/**
	 * Apply display mode + resolution reliably.
	 * Leaving exclusive fullscreen often fails with ApplySettings alone; this also
	 * reshapes the game window and avoids windowed-at-desktop-size looking stuck.
	 * @return The resolution actually applied (may be shrunk for windowed).
	 */
	FIntPoint ApplyDisplayMode(EWindowMode::Type Mode, FIntPoint Resolution);

	void ApplyExtraGraphicsSettings();
	void ApplyAudioSettings();

protected:
	UPROPERTY(config)
	bool bMotionBlurEnabled = false;

	UPROPERTY(config)
	int32 DemoSettingsVersion = 0;

	/** 0 Low .. 3 Epic. Stored separately because mixed ResolutionQuality makes GetOverallScalabilityLevel() return -1. */
	UPROPERTY(config)
	int32 PreferredOverallQuality = 2;

	UPROPERTY(config)
	float MasterVolume = 1.f;

	UPROPERTY(config)
	float MusicVolume = 1.f;

	UPROPERTY(config)
	float SFXVolume = 1.f;

	static constexpr int32 CurrentDemoSettingsVersion = 4;
};
