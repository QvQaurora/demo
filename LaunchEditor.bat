@echo off
setlocal
chcp 65001 >nul

set "UE_EDITOR=C:\Program Files\Epic Games\UE_5.8\Engine\Binaries\Win64\UnrealEditor.exe"
set "PROJECT=%~dp0demo.uproject"
set "ROOT_INI=%~dp0Saved\Config\WindowsEditor\EditorPerProjectUserSettings.ini"
set "FIX_PS1=%~dp0Tools\FixEditorWindow.ps1"

if not exist "%UE_EDITOR%" (
    echo 找不到 Unreal Editor 5.8:
    echo %UE_EDITOR%
    pause
    exit /b 1
)

if not exist "%PROJECT%" (
    echo 找不到项目文件:
    echo %PROJECT%
    pause
    exit /b 1
)

echo.
echo ========================================
echo  启动 demo ^| UE 5.8 + DX11 + MCP
echo ========================================
echo.
echo 若弹出「损坏的映像」或错误 0xc0e90002：
echo   这是 Windows「智能应用控制」拦截了本地编译的 DLL。
echo   请打开：
echo   设置 - 隐私和安全性 - Windows 安全中心
echo   - 应用和浏览器控制 - 智能应用控制设置
echo   将其关闭，然后重新运行本脚本。
echo.

rem Prefill RootWindow before launch, then keep correcting HWND for a while
rem because UE re-applies near-fullscreen size after layout load.
if exist "%FIX_PS1%" (
    powershell -NoProfile -ExecutionPolicy Bypass -File "%FIX_PS1%" -IniPath "%ROOT_INI%"
)

start "" "%UE_EDITOR%" "%PROJECT%" -d3d11 -ModelContextProtocolStartServer

if exist "%FIX_PS1%" (
    start "" /MIN powershell -NoProfile -ExecutionPolicy Bypass -File "%FIX_PS1%" -IniPath "%ROOT_INI%" -ApplyToRunningEditor -WatchSeconds 60
)

endlocal
