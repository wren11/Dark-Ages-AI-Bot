@echo off
setlocal enabledelayedexpansion

echo ===== Dark Ages AI Bot Proxy Setup =====

:: Ask for game directory
set /p GAME_DIR="Enter the path to your Dark Ages game directory: "

if not exist "%GAME_DIR%" (
    echo Error: Game directory does not exist!
    pause
    exit /b 1
)

:: Check if proxy DLL exists
set PROXY_DLL="bin\Release\dinput8.dll"
set BOT_DLL="bin\Release\pop.dll"

if not exist %PROXY_DLL% (
    echo Error: Proxy DLL not found! Did you build the solution?
    pause
    exit /b 1
)

if not exist %BOT_DLL% (
    echo Error: Bot DLL not found! Did you build the solution?
    pause
    exit /b 1
)

:: Check if original DLL exists and back it up
if exist "%GAME_DIR%\dinput8.dll" (
    if not exist "%GAME_DIR%\original_dinput8.dll" (
        echo Backing up original dinput8.dll...
        copy "%GAME_DIR%\dinput8.dll" "%GAME_DIR%\original_dinput8.dll"
    )
)

:: Copy our DLLs
echo Copying proxy DLL to game directory...
copy %PROXY_DLL% "%GAME_DIR%\dinput8.dll"

echo Copying bot DLL to game directory...
copy %BOT_DLL% "%GAME_DIR%\pop.dll"

:: Create required directories
if not exist "%GAME_DIR%\logs" mkdir "%GAME_DIR%\logs"
if not exist "%GAME_DIR%\config" mkdir "%GAME_DIR%\config"
if not exist "%GAME_DIR%\data" mkdir "%GAME_DIR%\data"

echo ===== Setup Complete =====
echo The bot will now load automatically when you start Dark Ages!
echo To uninstall, simply delete dinput8.dll and pop.dll from the game directory.
echo If you want to restore the original DLL, rename original_dinput8.dll back to dinput8.dll.
pause
