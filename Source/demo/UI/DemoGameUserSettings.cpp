// Copyright Epic Games, Inc. All Rights Reserved.

#include "DemoGameUserSettings.h"
#include "AudioDevice.h"
#include "Engine/Engine.h"
#include "Engine/GameViewportClient.h"
#include "Framework/Application/SlateApplication.h"
#include "HAL/IConsoleManager.h"
#include "Widgets/SWindow.h"

UDemoGameUserSettings::UDemoGameUserSettings()
{
	bUseVSync = true;
}

void UDemoGameUserSettings::SetToDefaults()
{
	Super::SetToDefaults();
	ApplyPreferredDefaults();
	MasterVolume = 1.f;
	MusicVolume = 1.f;
	SFXVolume = 1.f;
	DemoSettingsVersion = CurrentDemoSettingsVersion;
}

void UDemoGameUserSettings::ApplyPreferredDefaults()
{
	bUseVSync = true;
	bMotionBlurEnabled = false;
	PreferredOverallQuality = 2;

	// True windowed mode (not borderless / maximized).
	SetFullscreenMode(EWindowMode::Windowed);
	SetScreenResolution(FIntPoint(1920, 1080));
	SetOverallScalabilityLevel(PreferredOverallQuality);
	SetResolutionScaleValueEx(100.f);
	SetFrameRateLimit(0.f);
}

void UDemoGameUserSettings::LoadSettings(bool bForceReload)
{
	Super::LoadSettings(bForceReload);

	if (DemoSettingsVersion < CurrentDemoSettingsVersion)
	{
		ApplyPreferredDefaults();
		DemoSettingsVersion = CurrentDemoSettingsVersion;
		SaveSettings();
	}
}

UDemoGameUserSettings* UDemoGameUserSettings::GetDemoGameUserSettings()
{
	return Cast<UDemoGameUserSettings>(UGameUserSettings::GetGameUserSettings());
}

void UDemoGameUserSettings::SetMotionBlurEnabled(bool bEnabled)
{
	bMotionBlurEnabled = bEnabled;
}

void UDemoGameUserSettings::SetMasterVolume(float Volume)
{
	MasterVolume = FMath::Clamp(Volume, 0.f, 1.f);
}

void UDemoGameUserSettings::SetMusicVolume(float Volume)
{
	MusicVolume = FMath::Clamp(Volume, 0.f, 1.f);
}

void UDemoGameUserSettings::SetSFXVolume(float Volume)
{
	SFXVolume = FMath::Clamp(Volume, 0.f, 1.f);
}

void UDemoGameUserSettings::SetPreferredOverallQuality(int32 Level)
{
	PreferredOverallQuality = FMath::Clamp(Level, 0, 3);
}

FIntPoint UDemoGameUserSettings::ApplyDisplayMode(EWindowMode::Type Mode, FIntPoint Resolution)
{
	if (Resolution.X <= 0 || Resolution.Y <= 0)
	{
		Resolution = FIntPoint(1920, 1080);
	}

	// Windowed at native desktop size looks identical to fullscreen and often "won't switch back".
	if (Mode == EWindowMode::Windowed && FSlateApplication::IsInitialized())
	{
		FDisplayMetrics Metrics;
		FSlateApplication::Get().GetDisplayMetrics(Metrics);
		const int32 DisplayW = Metrics.PrimaryDisplayWidth;
		const int32 DisplayH = Metrics.PrimaryDisplayHeight;
		if (DisplayW > 0 && DisplayH > 0 &&
			Resolution.X >= (DisplayW - 32) && Resolution.Y >= (DisplayH - 32))
		{
			Resolution.X = FMath::Clamp(DisplayW * 3 / 4, 1280, 1920);
			Resolution.Y = FMath::Clamp(DisplayH * 3 / 4, 720, 1080);
			if (Resolution.X >= DisplayW)
			{
				Resolution.X = FMath::Max(1280, DisplayW - 120);
			}
			if (Resolution.Y >= DisplayH)
			{
				Resolution.Y = FMath::Max(720, DisplayH - 120);
			}
		}
	}

	SetScreenResolution(Resolution);
	SetFullscreenMode(Mode);
	ApplyResolutionSettings(false);
	ConfirmVideoMode();

	if (GEngine && GEngine->GameViewport)
	{
		if (TSharedPtr<SWindow> Window = GEngine->GameViewport->GetWindow())
		{
			Window->SetWindowMode(Mode);
			if (Mode == EWindowMode::Windowed)
			{
				FVector2D NewPos = Window->GetPositionInScreen();
				const FVector2D NewSize(Resolution.X, Resolution.Y);
				if (FSlateApplication::IsInitialized())
				{
					FDisplayMetrics Metrics;
					FSlateApplication::Get().GetCachedDisplayMetrics(Metrics);
					const FVector2D WorkTopLeft(
						static_cast<float>(Metrics.PrimaryDisplayWorkAreaRect.Left),
						static_cast<float>(Metrics.PrimaryDisplayWorkAreaRect.Top));
					const FVector2D WorkSize(
						static_cast<float>(Metrics.PrimaryDisplayWorkAreaRect.Right - Metrics.PrimaryDisplayWorkAreaRect.Left),
						static_cast<float>(Metrics.PrimaryDisplayWorkAreaRect.Bottom - Metrics.PrimaryDisplayWorkAreaRect.Top));
					NewPos = WorkTopLeft + (WorkSize - NewSize) * 0.5f;
				}
				Window->Restore();
				Window->ReshapeWindow(NewPos, NewSize);
			}
		}
	}

	// Belt-and-suspenders: r.SetRes is what the engine uses for mode switches.
	if (GEngine)
	{
		const TCHAR* Suffix =
			(Mode == EWindowMode::Fullscreen) ? TEXT("f") :
			(Mode == EWindowMode::WindowedFullscreen) ? TEXT("wf") :
			TEXT("w");
		GEngine->Exec(nullptr, *FString::Printf(TEXT("r.SetRes %dx%d%s"), Resolution.X, Resolution.Y, Suffix));
	}

	SaveSettings();
	return Resolution;
}

void UDemoGameUserSettings::ApplyExtraGraphicsSettings()
{
	if (IConsoleVariable* MotionBlurQuality = IConsoleManager::Get().FindConsoleVariable(TEXT("r.MotionBlurQuality")))
	{
		MotionBlurQuality->Set(bMotionBlurEnabled ? 4 : 0, ECVF_SetByGameSetting);
	}
	if (IConsoleVariable* MotionBlurFeature = IConsoleManager::Get().FindConsoleVariable(TEXT("r.DefaultFeature.MotionBlur")))
	{
		MotionBlurFeature->Set(bMotionBlurEnabled ? 1 : 0, ECVF_SetByGameSetting);
	}
}

void UDemoGameUserSettings::ApplyAudioSettings()
{
	if (!GEngine)
	{
		return;
	}

	if (FAudioDevice* AudioDevice = GEngine->GetMainAudioDeviceRaw())
	{
		AudioDevice->SetTransientPrimaryVolume(MasterVolume);
	}
}

void UDemoGameUserSettings::ApplyNonResolutionSettings()
{
	Super::ApplyNonResolutionSettings();
	ApplyExtraGraphicsSettings();
	ApplyAudioSettings();
}

void UDemoGameUserSettings::ApplySettings(bool bCheckForCommandLineOverrides)
{
	Super::ApplySettings(bCheckForCommandLineOverrides);
	ApplyExtraGraphicsSettings();
	ApplyAudioSettings();
}
