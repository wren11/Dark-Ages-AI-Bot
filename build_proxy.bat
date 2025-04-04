@echo off
setlocal enabledelayedexpansion

echo ===== Building Dark Ages AI Bot Proxy DLL =====

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

REM Create the proxy directory if it doesn't exist
if not exist "src\proxy" mkdir src\proxy

REM Create the proxy DLL file
echo Creating proxy DLL implementation file...
(
echo #include ^<windows.h^>
echo #include ^<string^>
echo #include ^<filesystem^>
echo #include ^<fstream^>
echo.
echo // Function prototypes for dinput8.dll exports
echo extern "C" {
echo     typedef HRESULT (WINAPI *DirectInput8Create_t)(HINSTANCE, DWORD, REFIID, LPVOID*, LPUNKNOWN);
echo     typedef HRESULT (WINAPI *DllCanUnloadNow_t)(void);
echo     typedef HRESULT (WINAPI *DllGetClassObject_t)(REFCLSID, REFIID, LPVOID*);
echo     typedef HRESULT (WINAPI *DllRegisterServer_t)(void);
echo     typedef HRESULT (WINAPI *DllUnregisterServer_t)(void);
echo }
echo.
echo // Global variables
echo HMODULE g_hOriginalDLL = nullptr;
echo HMODULE g_hBotDLL = nullptr;
echo bool g_bBotInitialized = false;
echo.
echo // Function pointers for the original DLL functions
echo DirectInput8Create_t g_pfnDirectInput8Create = nullptr;
echo DllCanUnloadNow_t g_pfnDllCanUnloadNow = nullptr;
echo DllGetClassObject_t g_pfnDllGetClassObject = nullptr;
echo DllRegisterServer_t g_pfnDllRegisterServer = nullptr;
echo DllUnregisterServer_t g_pfnDllUnregisterServer = nullptr;
echo.
echo // Simple logging function
echo void LogMessage(const std::string^& message) {
echo     try {
echo         std::filesystem::path logPath = std::filesystem::path("logs");
echo         
echo         if (!std::filesystem::exists(logPath)) {
echo             std::filesystem::create_directories(logPath);
echo         }
echo         
echo         std::ofstream logFile(logPath / "proxy.log", std::ios::app);
echo         logFile ^<^< "[Proxy] " ^<^< message ^<^< std::endl;
echo     } catch (...) {
echo         // Ignore logging errors
echo     }
echo }
echo.
echo // Load the original dinput8.dll
echo bool LoadOriginalDLL() {
echo     if (g_hOriginalDLL) return true;
echo     
echo     try {
echo         // Get the path to our proxy DLL
echo         char proxyPath[MAX_PATH];
echo         GetModuleFileNameA(GetModuleHandleA("dinput8.dll"), proxyPath, MAX_PATH);
echo         std::filesystem::path dllPath = proxyPath;
echo         std::filesystem::path gamePath = dllPath.parent_path();
echo         
echo         // Try to load the renamed original DLL first
echo         std::filesystem::path originalDllPath = gamePath / "original_dinput8.dll";
echo         LogMessage("Looking for original DLL at: " + originalDllPath.string());
echo         
echo         g_hOriginalDLL = LoadLibraryA(originalDllPath.string().c_str());
echo         if (!g_hOriginalDLL) {
echo             // If we can't find the renamed DLL, try the system one
echo             char systemPath[MAX_PATH];
echo             GetSystemDirectoryA(systemPath, MAX_PATH);
echo             originalDllPath = std::filesystem::path(systemPath) / "dinput8.dll";
echo             
echo             LogMessage("Trying system DLL at: " + originalDllPath.string());
echo             g_hOriginalDLL = LoadLibraryA(originalDllPath.string().c_str());
echo             
echo             if (!g_hOriginalDLL) {
echo                 LogMessage("Failed to load original dinput8.dll");
echo                 return false;
echo             }
echo         }
echo         
echo         // Get the function pointers from the original DLL
echo         g_pfnDirectInput8Create = (DirectInput8Create_t)GetProcAddress(g_hOriginalDLL, "DirectInput8Create");
echo         g_pfnDllCanUnloadNow = (DllCanUnloadNow_t)GetProcAddress(g_hOriginalDLL, "DllCanUnloadNow");
echo         g_pfnDllGetClassObject = (DllGetClassObject_t)GetProcAddress(g_hOriginalDLL, "DllGetClassObject");
echo         g_pfnDllRegisterServer = (DllRegisterServer_t)GetProcAddress(g_hOriginalDLL, "DllRegisterServer");
echo         g_pfnDllUnregisterServer = (DllUnregisterServer_t)GetProcAddress(g_hOriginalDLL, "DllUnregisterServer");
echo         
echo         LogMessage("Successfully loaded original DLL");
echo         return true;
echo     }
echo     catch (const std::exception^& ex) {
echo         LogMessage("Exception in LoadOriginalDLL: " + std::string(ex.what()));
echo         return false;
echo     }
echo     catch (...) {
echo         LogMessage("Unknown exception in LoadOriginalDLL");
echo         return false;
echo     }
echo }
echo.
echo // Initialize our bot DLL
echo bool InitializeBot() {
echo     if (g_bBotInitialized) return true;
echo     
echo     try {
echo         // Get the full path to the game directory
echo         char gamePath[MAX_PATH];
echo         GetModuleFileNameA(nullptr, gamePath, MAX_PATH);
echo         std::filesystem::path gameDir = std::filesystem::path(gamePath).parent_path();
echo         
echo         // Create path to our bot DLL
echo         std::filesystem::path botDllPath = gameDir / "pop.dll";
echo         LogMessage("Looking for bot DLL at: " + botDllPath.string());
echo         
echo         // Load our bot DLL
echo         g_hBotDLL = LoadLibraryA(botDllPath.string().c_str());
echo         if (!g_hBotDLL) {
echo             LogMessage("Failed to load bot DLL. Error code: " + std::to_string(GetLastError()));
echo             return false;
echo         }
echo         
echo         LogMessage("Successfully loaded bot DLL");
echo         g_bBotInitialized = true;
echo         return true;
echo     }
echo     catch (const std::exception^& ex) {
echo         LogMessage("Exception in InitializeBot: " + std::string(ex.what()));
echo         return false;
echo     }
echo     catch (...) {
echo         LogMessage("Unknown exception in InitializeBot");
echo         return false;
echo     }
echo }
echo.
echo // DLL entry point
echo BOOL APIENTRY DllMain(HMODULE hModule, DWORD ul_reason_for_call, LPVOID lpReserved) {
echo     switch (ul_reason_for_call) {
echo         case DLL_PROCESS_ATTACH:
echo             // Disable thread notifications to improve performance
echo             DisableThreadLibraryCalls(hModule);
echo             
echo             // Load the original DLL first
echo             if (!LoadOriginalDLL()) {
echo                 return FALSE;
echo             }
echo             
echo             // Initialize the bot in a separate thread to avoid blocking
echo             CreateThread(NULL, 0, 
echo                 [](LPVOID) -^> DWORD { 
echo                     // Wait a bit for the game to initialize
echo                     Sleep(1000);
echo                     InitializeBot();
echo                     return 0;
echo                 }, 
echo                 NULL, 0, NULL);
echo             break;
echo             
echo         case DLL_PROCESS_DETACH:
echo             // Clean up when unloaded
echo             if (lpReserved != nullptr) {
echo                 break; // Do not clean up if process termination
echo             }
echo             
echo             // Free the bot DLL
echo             if (g_hBotDLL) {
echo                 FreeLibrary(g_hBotDLL);
echo                 g_hBotDLL = nullptr;
echo             }
echo             
echo             // Free the original DLL
echo             if (g_hOriginalDLL) {
echo                 FreeLibrary(g_hOriginalDLL);
echo                 g_hOriginalDLL = nullptr;
echo             }
echo             break;
echo     }
echo     
echo     return TRUE;
echo }
echo.
echo // Export functions that forward to the original DLL
echo extern "C" {
echo.
echo // DirectInput8Create function
echo __declspec(dllexport) HRESULT WINAPI DirectInput8Create(
echo     HINSTANCE hinst, DWORD dwVersion, REFIID riidltf, LPVOID* ppvOut, LPUNKNOWN punkOuter) {
echo     
echo     if (!g_pfnDirectInput8Create) {
echo         if (!LoadOriginalDLL()) {
echo             return E_FAIL;
echo         }
echo     }
echo     
echo     return g_pfnDirectInput8Create(hinst, dwVersion, riidltf, ppvOut, punkOuter);
echo }
echo.
echo // DllCanUnloadNow function
echo __declspec(dllexport) HRESULT WINAPI DllCanUnloadNow(void) {
echo     if (!g_pfnDllCanUnloadNow) {
echo         if (!LoadOriginalDLL()) {
echo             return S_FALSE;
echo         }
echo     }
echo     
echo     return g_pfnDllCanUnloadNow();
echo }
echo.
echo // DllGetClassObject function
echo __declspec(dllexport) HRESULT WINAPI DllGetClassObject(REFCLSID rclsid, REFIID riid, LPVOID* ppv) {
echo     if (!g_pfnDllGetClassObject) {
echo         if (!LoadOriginalDLL()) {
echo             return E_FAIL;
echo         }
echo     }
echo     
echo     return g_pfnDllGetClassObject(rclsid, riid, ppv);
echo }
echo.
echo // DllRegisterServer function
echo __declspec(dllexport) HRESULT WINAPI DllRegisterServer(void) {
echo     if (!g_pfnDllRegisterServer) {
echo         if (!LoadOriginalDLL()) {
echo             return E_FAIL;
echo         }
echo     }
echo     
echo     return g_pfnDllRegisterServer();
echo }
echo.
echo // DllUnregisterServer function
echo __declspec(dllexport) HRESULT WINAPI DllUnregisterServer(void) {
echo     if (!g_pfnDllUnregisterServer) {
echo         if (!LoadOriginalDLL()) {
echo             return E_FAIL;
echo         }
echo     }
echo     
echo     return g_pfnDllUnregisterServer();
echo }
echo.
echo } // extern "C"
) > "src\proxy\dinput8_proxy.cpp"

REM Create the module definition file
echo Creating module definition file...
(
echo LIBRARY dinput8
echo EXPORTS
echo     DirectInput8Create @1
echo     DllCanUnloadNow @2 PRIVATE
echo     DllGetClassObject @3 PRIVATE
echo     DllRegisterServer @4 PRIVATE
echo     DllUnregisterServer @5 PRIVATE
) > "src\proxy\dinput8.def"

REM Compile the proxy DLL directly with cl.exe
echo Compiling proxy DLL...
cl.exe /nologo /EHsc /MD /std:c++17 /O2 /Fe:"bin\Release\dinput8.dll" src\proxy\dinput8_proxy.cpp /link /DLL /DEF:src\proxy\dinput8.def

if %ERRORLEVEL% neq 0 (
    echo ERROR: Failed to build proxy DLL
    exit /b 1
)

echo.
echo ===== Proxy DLL built successfully! =====
echo.
echo The proxy DLL has been built to: bin\Release\dinput8.dll
echo.
echo To use the proxy:
echo 1. Build your main pop.dll using Visual Studio
echo 2. Copy both dinput8.dll and pop.dll to your Dark Ages game directory
echo 3. Rename the original dinput8.dll (if any) to original_dinput8.dll
echo.
echo The bot will automatically load when you start the game!

exit /b 0
