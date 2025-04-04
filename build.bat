@echo off
setlocal enabledelayedexpansion

:: Set variables
set BUILD_DIR=build
set INSTALL_DIR=dist
set BUILD_TYPE=Release
set PROXY_DLL=dinput8

echo ===== Dark Ages AI Bot Build Script =====

:: Check if CMake is installed
where cmake >nul 2>nul
if %ERRORLEVEL% neq 0 (
    echo ERROR: CMake not found. Please install CMake and add it to your PATH.
    exit /b 1
)

:: Create directories
if not exist %BUILD_DIR% mkdir %BUILD_DIR%
if not exist %INSTALL_DIR% mkdir %INSTALL_DIR%
if not exist external mkdir external

:: Check if Detours is present, if not, clone it
if not exist external\detours (
    echo Downloading Microsoft Detours...
    git clone https://github.com/microsoft/Detours.git external\detours
    if %ERRORLEVEL% neq 0 (
        echo Failed to download Detours. Please download it manually to external\detours
    )
)

:: Check if JsonCpp is present, if not, clone it
if not exist external\json (
    echo Downloading JsonCpp...
    git clone https://github.com/open-source-parsers/jsoncpp.git external\json
    if %ERRORLEVEL% neq 0 (
        echo Failed to download JsonCpp. Please download it manually to external\json
    )
)

:: Configure CMake
echo Configuring CMake...
cd %BUILD_DIR%
cmake .. -DCMAKE_INSTALL_PREFIX=../%INSTALL_DIR% -DCMAKE_BUILD_TYPE=%BUILD_TYPE%

:: Build the project
echo Building project...
cmake --build . --config %BUILD_TYPE% -j 4

:: Install the project
echo Installing to %INSTALL_DIR%...
cmake --install . --config %BUILD_TYPE%

:: Copy required files for proxy DLL
echo Setting up proxy DLL...
if exist "..\%INSTALL_DIR%\bin\%PROXY_DLL%.dll" (
    echo Copying proxy DLL to game directory...
    
    :: Ask for game directory
    set /p GAME_DIR="Enter the path to your Dark Ages game directory (or press Enter to skip): "
    
    if not "!GAME_DIR!"=="" (
        :: Check if the directory exists
        if exist "!GAME_DIR!" (
            :: Check if original DLL exists
            if exist "!GAME_DIR!\%PROXY_DLL%.dll" (
                :: Backup original DLL if it hasn't been backed up
                if not exist "!GAME_DIR!\original_%PROXY_DLL%.dll" (
                    echo Backing up original !GAME_DIR!\%PROXY_DLL%.dll to !GAME_DIR!\original_%PROXY_DLL%.dll
                    copy "!GAME_DIR!\%PROXY_DLL%.dll" "!GAME_DIR!\original_%PROXY_DLL%.dll"
                )
            )
            
            :: Copy our proxy DLL and bot DLL
            echo Copying %PROXY_DLL%.dll to !GAME_DIR!
            copy "..\%INSTALL_DIR%\bin\%PROXY_DLL%.dll" "!GAME_DIR!"
            
            :: If we have a bot DLL, copy that too
            if exist "..\%INSTALL_DIR%\bin\DarkAgesAIBot.dll" (
                echo Copying DarkAgesAIBot.dll to !GAME_DIR!
                copy "..\%INSTALL_DIR%\bin\DarkAgesAIBot.dll" "!GAME_DIR!"
            )
            
            :: Create data directories in game folder
            if not exist "!GAME_DIR!\config" mkdir "!GAME_DIR!\config"
            if not exist "!GAME_DIR!\data" mkdir "!GAME_DIR!\data"
            if not exist "!GAME_DIR!\logs" mkdir "!GAME_DIR!\logs"
            
            echo Deployment complete!
        ) else (
            echo Game directory not found: !GAME_DIR!
        )
    )
)

cd ..
echo Build completed successfully!
echo The output files are in the %INSTALL_DIR% directory.

echo ===== Done =====
exit /b 0
