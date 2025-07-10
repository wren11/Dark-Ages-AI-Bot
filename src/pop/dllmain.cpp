// ReSharper disable CppClangTidyCertErr33C
#include "pch.h"
#include "intercept_manager.h"
#include "io.h"
#include "network_communicator.h"

static bool hooksApplied = false;
static std::thread hookThread;

void InitializeConsole()
{
	AllocConsole();

	HANDLE consoleOutput = GetStdHandle(STD_OUTPUT_HANDLE);
	int systemOutput = _open_osfhandle(reinterpret_cast<intptr_t>(consoleOutput), _O_TEXT);
	FILE *COutputHandle = _fdopen(systemOutput, "w");

	std::ios::sync_with_stdio(true);
	freopen_s(&COutputHandle, "CONOUT$", "w", stdout);
	std::cout.clear();
	std::cout.rdbuf(std::cout.rdbuf());
}

BOOL APIENTRY DllMain(HMODULE hModule, DWORD ul_reason_for_call, LPVOID lpReserved)
{
	switch (ul_reason_for_call)
	{
	case DLL_PROCESS_ATTACH:
		DisableThreadLibraryCalls(hModule);
		InitializeConsole();
		initialize_shared_memory();

		if (hooksApplied && hookThread.joinable())
		{
			hookThread.join();
		}

		hookThread = std::thread([]()
								 {
				intercept_manager::Initialize();
				intercept_manager::AttachHook();
				hooksApplied = true; });
		break;

	case DLL_PROCESS_DETACH:
		FreeConsole();
		cleanup_shared_memory();
		if (hooksApplied && hookThread.joinable())
		{
			hookThread = std::thread([]()
									 {
					intercept_manager::RemoveHook();
					hooksApplied = false; });
			hookThread.join();
		}
		break;

	case DLL_THREAD_ATTACH:
	case DLL_THREAD_DETACH:
		break;
	default:;
	}
	return TRUE;
}
