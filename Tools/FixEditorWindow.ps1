param(
	[Parameter(Mandatory = $true)]
	[string]$IniPath,
	[double]$X = 160,
	[double]$Y = 80,
	[double]$W = 1600,
	[double]$H = 900,
	[switch]$ApplyToRunningEditor,
	[int]$WatchSeconds = 45
)

function Write-RootWindowIni
{
	param([string]$Path, [double]$PosX, [double]$PosY, [double]$Width, [double]$Height)

	if (-not (Test-Path -LiteralPath $Path))
	{
		$dir = Split-Path -Parent $Path
		if (-not (Test-Path -LiteralPath $dir))
		{
			New-Item -ItemType Directory -Path $dir -Force | Out-Null
		}
		Set-Content -LiteralPath $Path -Value "" -Encoding UTF8
	}

	$section = @"
[RootWindow]
ScreenPosition=X=$("{0:0.000}" -f $PosX) Y=$("{0:0.000}" -f $PosY)
WindowSize=X=$("{0:0.000}" -f $Width) Y=$("{0:0.000}" -f $Height)
InitiallyMaximized=False
"@

	$text = [IO.File]::ReadAllText($Path)
	$pattern = '(?ms)^\[RootWindow\]\r?\n(?:.*?\r?\n)*?(?=^\[|\z)'
	if ([regex]::IsMatch($text, $pattern))
	{
		$text = [regex]::Replace($text, $pattern, ($section.TrimEnd() + "`r`n"))
	}
	else
	{
		if (-not $text.EndsWith("`n"))
		{
			$text += "`r`n"
		}
		$text += $section.TrimEnd() + "`r`n"
	}

	[IO.File]::WriteAllText($Path, $text)
}

function Ensure-UeWinType
{
	if (-not ("UeWin" -as [type]))
	{
		Add-Type -TypeDefinition @'
using System;
using System.Runtime.InteropServices;
using System.Text;
public static class UeWin {
  public const int SW_RESTORE = 9;
  public const int GWL_STYLE = -16;
  public const int WS_MAXIMIZE = 0x01000000;
  public const uint SWP_NOZORDER = 0x0004;
  public const uint SWP_SHOWWINDOW = 0x0040;
  public const uint SWP_FRAMECHANGED = 0x0020;

  [DllImport("user32.dll")] public static extern bool ShowWindow(IntPtr hWnd, int nCmdShow);
  [DllImport("user32.dll")] public static extern bool SetWindowPos(IntPtr hWnd, IntPtr hInsert, int X, int Y, int cx, int cy, uint uFlags);
  [DllImport("user32.dll")] public static extern bool IsWindowVisible(IntPtr hWnd);
  [DllImport("user32.dll")] public static extern bool GetWindowRect(IntPtr hWnd, out RECT r);
  [DllImport("user32.dll")] public static extern int GetWindowText(IntPtr hWnd, StringBuilder lp, int nMax);
  [DllImport("user32.dll")] public static extern bool EnumWindows(EnumProc lp, IntPtr lParam);
  [DllImport("user32.dll")] public static extern uint GetWindowThreadProcessId(IntPtr hWnd, out uint pid);
  [DllImport("user32.dll")] public static extern int GetWindowLong(IntPtr hWnd, int nIndex);
  [DllImport("user32.dll")] public static extern int SetWindowLong(IntPtr hWnd, int nIndex, int dwNewLong);
  [DllImport("user32.dll")] public static extern bool IsZoomed(IntPtr hWnd);
  [StructLayout(LayoutKind.Sequential)] public struct RECT { public int L, T, R, B; }
  public delegate bool EnumProc(IntPtr hWnd, IntPtr lParam);
}
'@
	}
}

function Find-UnrealEditorHwnd
{
	Ensure-UeWinType
	$procs = @(Get-Process UnrealEditor -ErrorAction SilentlyContinue)
	foreach ($proc in $procs)
	{
		$h = $proc.MainWindowHandle
		if ($h -eq [IntPtr]::Zero) { continue }
		if (-not [UeWin]::IsWindowVisible($h)) { continue }
		if ([string]::IsNullOrWhiteSpace($proc.MainWindowTitle)) { continue }

		$r = New-Object UeWin+RECT
		[void][UeWin]::GetWindowRect($h, [ref]$r)
		$w = $r.R - $r.L
		$ht = $r.B - $r.T
		# Skip minimized / not-yet-shown windows.
		if (($w -lt 200) -or ($ht -lt 100) -or ($r.L -lt -1000)) { continue }
		return $h
	}
	return [IntPtr]::Zero
}

function Force-EditorRect
{
	param([IntPtr]$Hwnd, [int]$PosX, [int]$PosY, [int]$Width, [int]$Height)

	Ensure-UeWinType
	[void][UeWin]::ShowWindow($Hwnd, [UeWin]::SW_RESTORE)
	$style = [UeWin]::GetWindowLong($Hwnd, [UeWin]::GWL_STYLE)
	if (($style -band [UeWin]::WS_MAXIMIZE) -ne 0)
	{
		[void][UeWin]::SetWindowLong($Hwnd, [UeWin]::GWL_STYLE, ($style -band (-bnot [UeWin]::WS_MAXIMIZE)))
	}
	[void][UeWin]::SetWindowPos(
		$Hwnd,
		[IntPtr]::Zero,
		$PosX, $PosY, $Width, $Height,
		([UeWin]::SWP_NOZORDER -bor [UeWin]::SWP_SHOWWINDOW -bor [UeWin]::SWP_FRAMECHANGED)
	)
}

function Test-EditorNeedsResize
{
	param([IntPtr]$Hwnd, [int]$Width, [int]$Height)

	Ensure-UeWinType
	if ([UeWin]::IsZoomed($Hwnd)) { return $true }
	$r = New-Object UeWin+RECT
	[void][UeWin]::GetWindowRect($Hwnd, [ref]$r)
	$w = $r.R - $r.L
	$ht = $r.B - $r.T
	# Treat near-fullscreen as wrong (within 80px of target mismatch).
	return (($w -gt ($Width + 80)) -or ($ht -gt ($Height + 80)) -or ($w -lt ($Width - 80)) -or ($ht -lt ($Height - 80)))
}

function Resize-UnrealEditorWindow
{
	param([int]$PosX, [int]$PosY, [int]$Width, [int]$Height, [int]$TimeoutSec = 120, [int]$WatchSec = 45)

	Ensure-UeWinType
	$deadline = (Get-Date).AddSeconds($TimeoutSec)
	$target = [IntPtr]::Zero

	while ((Get-Date) -lt $deadline)
	{
		$target = Find-UnrealEditorHwnd
		if ($target -ne [IntPtr]::Zero) { break }
		Start-Sleep -Milliseconds 400
	}

	if ($target -eq [IntPtr]::Zero)
	{
		Write-Host "Unreal Editor main window not found; skip resize."
		return
	}

	Force-EditorRect -Hwnd $target -PosX $PosX -PosY $PosY -Width $Width -Height $Height

	$watchUntil = (Get-Date).AddSeconds($WatchSec)
	$applied = 0
	while ((Get-Date) -lt $watchUntil)
	{
		Start-Sleep -Milliseconds 750
		$target = Find-UnrealEditorHwnd
		if ($target -eq [IntPtr]::Zero) { continue }
		if (Test-EditorNeedsResize -Hwnd $target -Width $Width -Height $Height)
		{
			Force-EditorRect -Hwnd $target -PosX $PosX -PosY $PosY -Width $Width -Height $Height
			$applied++
		}
	}

	Write-Host ("Resized Unreal Editor to {0}x{1} at {2},{3} (reapplied {4} times)" -f $Width, $Height, $PosX, $PosY, $applied)
}

Write-RootWindowIni -Path $IniPath -PosX $X -PosY $Y -Width $W -Height $H

if ($ApplyToRunningEditor)
{
	Resize-UnrealEditorWindow -PosX ([int]$X) -PosY ([int]$Y) -Width ([int]$W) -Height ([int]$H) -WatchSec $WatchSeconds
}
