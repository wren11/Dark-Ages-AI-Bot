@echo off
setlocal enabledelayedexpansion

echo ===== Building Dark Ages AI Bot DLLs =====

REM Detect Visual Studio installation
set "VSWHERE=%ProgramFiles(x86)%\Microsoft Visual Studio\Installer\vswhere.exe"
if not exist "!VSWHERE!" (
    echo ERROR: Visual Studio installation not found.
    echo Please make sure Visual Studio is installed with C++ workload.
    exit /b 1
)

REM Find the latest Visual Studio installation
for /f "usebackq tokens=*" %%i in (`"!VSWHERE!" -latest -products * -requires Microsoft.Component.MSBuild -property installationPath`) do (
    set "VS_PATH=%%i"
)

if not defined VS_PATH (
    echo ERROR: Could not find Visual Studio installation.
    exit /b 1
)

REM Set up the Visual Studio environment
set "VCVARS=!VS_PATH!\VC\Auxiliary\Build\vcvars32.bat"
if not exist "!VCVARS!" (
    echo ERROR: Visual Studio environment setup script not found.
    exit /b 1
)

REM Initialize the Visual Studio environment
echo Initializing Visual Studio environment...
call "!VCVARS!"

REM Create output directories
if not exist "bin" mkdir bin
if not exist "bin\Release" mkdir bin\Release

REM Build the main pop.dll
echo Building main pop.dll...
msbuild "src\pop\pop.vcxproj" /p:Configuration=Release /p:Platform=Win32 /p:OutDir="..\..\bin\Release\\" /t:Rebuild

if %ERRORLEVEL% neq 0 (
    echo ERROR: Failed to build pop.dll
    exit /b 1
)

REM Create the proxy project directory if it doesn't exist
if not exist "src\proxy" mkdir src\proxy

REM Create the proxy DLL project and source files
echo Setting up proxy DLL files...

REM Create the proxy DLL implementation file
if not exist "src\proxy\dinput8_proxy.cpp" (
    echo Creating proxy DLL implementation...
    (
    echo #include ^<windows.h^>
    echo #include ^<string^>
    echo #include ^<filesystem^>
    echo #include ^<fstream^>
    echo.
    echo // Function pointer types for the exports
    echo using DirectInput8Create_t = HRESULT(WINAPI^*^)(HINSTANCE, DWORD, REFIID, LPVOID^*, LPUNKNOWN^);
    echo using DllCanUnloadNow_t = HRESULT(WINAPI^*^)(^);
    echo using DllGetClassObject_t = HRESULT(WINAPI^*^)(REFCLSID, REFIID, LPVOID^*^);
    echo using DllRegisterServer_t = HRESULT(WINAPI^*^)(^);
    echo using DllUnregisterServer_t = HRESULT(WINAPI^*^)(^);
    echo.
    echo // Global variables
    echo HMODULE g_hOriginalDLL = nullptr;
    echo HMODULE g_hBotDLL = nullptr;
    echo bool g_bInitialized = false;
    echo.
    echo // Log file for debugging
    echo void LogMessage(const std::string^& message^) {
    echo     try {
    echo         std::filesystem::path logPath = std::filesystem::path("logs");
    echo         if (!std::filesystem::exists(logPath^)^) {
    echo             std::filesystem::create_directories(logPath^);
    echo         }
    echo         std::ofstream logFile(logPath / "proxy.log", std::ios::app^);
    echo         logFile ^<^< "[Proxy] " ^<^< message ^<^< std::endl;
    echo     } catch (...^) {
    echo         // Just swallow any errors during logging
    echo     }
    echo }
    echo.
    echo // Function to load the original DLL
    echo bool LoadOriginalDLL(^) {
    echo     if (g_hOriginalDLL^) return true;
    echo     
    echo     try {
    echo         // Get current module path
    echo         char proxyPath[MAX_PATH];
    echo         GetModuleFileNameA(GetModuleHandleA("dinput8.dll"^), proxyPath, MAX_PATH^);
    echo         
    echo         std::filesystem::path dllPath = proxyPath;
    echo         std::filesystem::path gamePath = dllPath.parent_path(^);
    echo         
    echo         // Try to load the renamed original DLL
    echo         std::filesystem::path originalPath = gamePath / "original_dinput8.dll";
    echo         
    echo         LogMessage("Looking for original DLL at: " + originalPath.string(^)^);
    echo         
    echo         g_hOriginalDLL = LoadLibraryA(originalPath.string(^).c_str(^)^);
    echo         if (!g_hOriginalDLL^) {
    echo             // If renamed DLL doesn't exist, try the system one
    echo             char systemPath[MAX_PATH];
    echo             GetSystemDirectoryA(systemPath, MAX_PATH^);
    echo             
    echo             originalPath = std::filesystem::path(systemPath^) / "dinput8.dll";
    echo             LogMessage("Trying system DLL at: " + originalPath.string(^)^);
    echo             
    echo             g_hOriginalDLL = LoadLibraryA(originalPath.string(^).c_str(^)^);
    echo             
    echo             if (!g_hOriginalDLL^) {
    echo                 LogMessage("Failed to load original dinput8.dll"^);
    echo                 return false;
    echo             }
    echo         }
    echo         
    echo         LogMessage("Successfully loaded original DLL"^);
    echo         return true;
    echo     }
    echo     catch (const std::exception^& ex^) {
    echo         LogMessage("Exception in LoadOriginalDLL: " + std::string(ex.what(^)^)^);
    echo         return false;
    echo     }
    echo     catch (...^) {
    echo         LogMessage("Unknown exception in LoadOriginalDLL"^);
    echo         return false;
    echo     }
    echo }
    echo.
    echo // Function to initialize our bot
    echo bool InitializeBot(^) {
    echo     if (g_bInitialized^) return true;
    echo     
    echo     try {
    echo         // Get the full path to our DLL
    echo         char proxyPath[MAX_PATH];
    echo         GetModuleFileNameA(GetModuleHandleA(NULL^), proxyPath, MAX_PATH^);
    echo         std::filesystem::path gamePath = std::filesystem::path(proxyPath^).parent_path(^);
    echo         
    echo         // Create path to the bot DLL (assumed to be in the same directory^)
    echo         std::filesystem::path botDllPath = gamePath / "pop.dll";
    echo         
    echo         LogMessage("Looking for bot DLL at: " + botDllPath.string(^)^);
    echo         
    echo         // Load the bot DLL
    echo         g_hBotDLL = LoadLibraryA(botDllPath.string(^).c_str(^)^);
    echo         
    echo         if (!g_hBotDLL^) {
    echo             LogMessage("Failed to load bot DLL. Error code: " + std::to_string(GetLastError(^)^)^);
    echo             return false;
    echo         }
    echo         
    echo         LogMessage("Successfully loaded bot DLL"^);
    echo         g_bInitialized = true;
    echo         return true;
    echo     }
    echo     catch (const std::exception^& ex^) {
    echo         LogMessage("Exception in InitializeBot: " + std::string(ex.what(^)^)^);
    echo         return false;
    echo     }
    echo     catch (...^) {
    echo         LogMessage("Unknown exception in InitializeBot"^);
    echo         return false;
    echo     }
    echo }
    echo.
    echo // DLL entry point
    echo BOOL APIENTRY DllMain(HMODULE hModule, DWORD ul_reason_for_call, LPVOID lpReserved^) {
    echo     switch (ul_reason_for_call^) {
    echo         case DLL_PROCESS_ATTACH:
    echo             DisableThreadLibraryCalls(hModule^);
    echo             
    echo             // Load the original DLL immediately
    echo             if (!LoadOriginalDLL(^)^) {
    echo                 return FALSE;
    echo             }
    echo             
    echo             // Initialize the bot in a separate thread to avoid blocking
    echo             CreateThread(NULL, 0, 
    echo                 [](LPVOID^) -^> DWORD { 
    echo                     Sleep(1000^); // Wait a bit to let the game initialize
    echo                     InitializeBot(^);
    echo                     return 0;
    echo                 }, 
    echo                 NULL, 0, NULL^);
    echo             break;
    echo             
    echo         case DLL_PROCESS_DETACH:
    echo             // Clean up
    echo             if (lpReserved != nullptr^) {
    echo                 break; // Don't clean up on process termination
    echo             }
    echo             
    echo             // Free our DLLs
    echo             if (g_hBotDLL^) {
    echo                 FreeLibrary(g_hBotDLL^);
    echo                 g_hBotDLL = nullptr;
    echo             }
    echo             
    echo             if (g_hOriginalDLL^) {
    echo                 FreeLibrary(g_hOriginalDLL^);
    echo                 g_hOriginalDLL = nullptr;
    echo             }
    echo             break;
    echo     }
    echo     
    echo     return TRUE;
    echo }
    echo.
    echo // Export functions that will be forwarded to the original DLL
    echo extern "C" {
    echo.
    echo // DirectInput8Create function
    echo __declspec(dllexport^) HRESULT WINAPI DirectInput8Create(
    echo     HINSTANCE hinst, DWORD dwVersion, REFIID riidltf, LPVOID^* ppvOut, LPUNKNOWN punkOuter^) {
    echo     
    echo     // Ensure original DLL is loaded
    echo     if (!g_hOriginalDLL ^&^& !LoadOriginalDLL(^)^) {
    echo         return E_FAIL;
    echo     }
    echo     
    echo     // Get the original function
    echo     auto original = reinterpret_cast^<DirectInput8Create_t^>(
    echo         GetProcAddress(g_hOriginalDLL, "DirectInput8Create"^)
    echo     ^);
    echo     
    echo     if (!original^) {
    echo         return E_FAIL;
    echo     }
    echo     
    echo     // Call the original function
    echo     return original(hinst, dwVersion, riidltf, ppvOut, punkOuter^);
    echo }
    echo.
    echo // DllCanUnloadNow function
    echo __declspec(dllexport^) HRESULT WINAPI DllCanUnloadNow(^) {
    echo     // Ensure original DLL is loaded
    echo     if (!g_hOriginalDLL ^&^& !LoadOriginalDLL(^)^) {
    echo         return S_FALSE;
    echo     }
    echo     
    echo     // Get the original function
    echo     auto original = reinterpret_cast^<DllCanUnloadNow_t^>(
    echo         GetProcAddress(g_hOriginalDLL, "DllCanUnloadNow"^)
    echo     ^);
    echo     
    echo     if (!original^) {
    echo         return S_FALSE;
    echo     }
    echo     
    echo     // Call the original function
    echo     return original(^);
    echo }
    echo.
    echo // DllGetClassObject function
    echo __declspec(dllexport^) HRESULT WINAPI DllGetClassObject(REFCLSID rclsid, REFIID riid, LPVOID^* ppv^) {
    echo     // Ensure original DLL is loaded
    echo     if (!g_hOriginalDLL ^&^& !LoadOriginalDLL(^)^) {
    echo         return E_FAIL;
    echo     }
    echo     
    echo     // Get the original function
    echo     auto original = reinterpret_cast^<DllGetClassObject_t^>(
    echo         GetProcAddress(g_hOriginalDLL, "DllGetClassObject"^)
    echo     ^);
    echo     
    echo     if (!original^) {
    echo         return E_FAIL;
    echo     }
    echo     
    echo     // Call the original function
    echo     return original(rclsid, riid, ppv^);
    echo }
    echo.
    echo // DllRegisterServer function
    echo __declspec(dllexport^) HRESULT WINAPI DllRegisterServer(^) {
    echo     // Ensure original DLL is loaded
    echo     if (!g_hOriginalDLL ^&^& !LoadOriginalDLL(^)^) {
    echo         return E_FAIL;
    echo     }
    echo     
    echo     // Get the original function
    echo     auto original = reinterpret_cast^<DllRegisterServer_t^>(
    echo         GetProcAddress(g_hOriginalDLL, "DllRegisterServer"^)
    echo     ^);
    echo     
    echo     if (!original^) {
    echo         return E_FAIL;
    echo     }
    echo     
    echo     // Call the original function
    echo     return original(^);
    echo }
    echo.
    echo // DllUnregisterServer function
    echo __declspec(dllexport^) HRESULT WINAPI DllUnregisterServer(^) {
    echo     // Ensure original DLL is loaded
    echo     if (!g_hOriginalDLL ^&^& !LoadOriginalDLL(^)^) {
    echo         return E_FAIL;
    echo     }
    echo     
    echo     // Get the original function
    echo     auto original = reinterpret_cast^<DllUnregisterServer_t^>(
    echo         GetProcAddress(g_hOriginalDLL, "DllUnregisterServer"^)
    echo     ^);
    echo     
    echo     if (!original^) {
    echo         return E_FAIL;
    echo     }
    echo     
    echo     // Call the original function
    echo     return original(^);
    echo }
    echo.
    echo } // extern "C"
    ) > "src\proxy\dinput8_proxy.cpp"
)

REM Create the module definition file
if not exist "src\proxy\dinput8.def" (
    echo Creating proxy DLL .def file...
    (
    echo LIBRARY dinput8
    echo EXPORTS
    echo     DirectInput8Create @1
    echo     DllCanUnloadNow @2 PRIVATE
    echo     DllGetClassObject @3 PRIVATE
    echo     DllRegisterServer @4 PRIVATE
    echo     DllUnregisterServer @5 PRIVATE
    ) > "src\proxy\dinput8.def"
)

REM Compile the proxy DLL
echo Building proxy DLL (dinput8.dll)...
cl.exe /D_USRDLL /D_WINDLL /O2 /Oi /Oy /GL /FIwindows.h /EHsc /MD /Zc:wchar_t /Zc:forScope /Gd /TP src\proxy\dinput8_proxy.cpp /Fe:bin\Release\dinput8.dll /link /OUT:bin\Release\dinput8.dll /DLL /DEF:src\proxy\dinput8.def user32.lib kernel32.lib

if %ERRORLEVEL% neq 0 (
    echo ERROR: Failed to build proxy DLL
    exit /b 1
)

echo.
echo ===== Build completed successfully! =====
echo.
echo The following DLLs have been built:
echo  - Main Bot DLL: bin\Release\pop.dll
echo  - Proxy DLL: bin\Release\dinput8.dll
echo.
echo To install:
echo 1. Copy both DLLs to your Dark Ages game directory
echo 2. Rename the original dinput8.dll to original_dinput8.dll (if it exists)
echo.
echo The bot will automatically load when you start the game!

exit /b 0
