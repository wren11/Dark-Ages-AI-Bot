#include <algorithm>
#include <iostream>
#include <string>
#include <tlhelp32.h>
#include <vector>
#include <windows.h>

static std::wstring dllPath1;
static std::wstring dllPath2;

void handleProcess(DWORD processID);

bool InjectDLL(DWORD processID, const std::wstring& dllPath) {
    HANDLE hProcess = OpenProcess(PROCESS_ALL_ACCESS, FALSE, processID);
    if (!hProcess) {
        std::cerr << "Failed to open target process for injection." << std::endl;
        return false;
    }

    // Allocate memory in the target process for the DLL path
    LPVOID allocMem = VirtualAllocEx(hProcess, nullptr, (dllPath.size() + 1) * sizeof(wchar_t), MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE);
    if (!allocMem) {
        std::cerr << "Failed to allocate memory in target process." << std::endl;
        CloseHandle(hProcess);
        return false;
    }

    // Write the DLL path to the allocated memory
    if (!WriteProcessMemory(hProcess, allocMem, dllPath.c_str(), (dllPath.size() + 1) * sizeof(wchar_t), nullptr)) {
        std::cerr << "Failed to write DLL path to target process memory." << std::endl;
        VirtualFreeEx(hProcess, allocMem, 0, MEM_RELEASE);
        CloseHandle(hProcess);
        return false;
    }

    // Create a remote thread that loads the DLL
    HANDLE hThread = CreateRemoteThread(hProcess, nullptr, 0, (LPTHREAD_START_ROUTINE)LoadLibraryW, allocMem, 0, nullptr);
    if (!hThread) {
        std::cerr << "Failed to create remote thread in target process." << std::endl;
        VirtualFreeEx(hProcess, allocMem, 0, MEM_RELEASE);
        CloseHandle(hProcess);
        return false;
    }

    // Wait for the remote thread to terminate
    WaitForSingleObject(hThread, INFINITE);

    // Clean up
    VirtualFreeEx(hProcess, allocMem, 0, MEM_RELEASE);
    CloseHandle(hThread);
    CloseHandle(hProcess);

    return true;
}

bool EjectDLL(DWORD processID, const std::wstring& dllPath) {
    HANDLE hProcess = OpenProcess(PROCESS_ALL_ACCESS, FALSE, processID);
    if (!hProcess) {
        std::cerr << "Failed to open target process for ejection." << std::endl;
        return false;
    }

    MODULEENTRY32W me32;
    HANDLE hModuleSnap = CreateToolhelp32Snapshot(TH32CS_SNAPMODULE, processID);
    if (hModuleSnap == INVALID_HANDLE_VALUE) {
        CloseHandle(hProcess);
        return false;
    }

    me32.dwSize = sizeof(MODULEENTRY32W);
    BOOL bModule = Module32FirstW(hModuleSnap, &me32);
    while (bModule) {
        if (std::wstring(me32.szModule) == dllPath) {
            // Found the DLL. Now eject it.
            HANDLE hThread = CreateRemoteThread(hProcess, nullptr, 0, (LPTHREAD_START_ROUTINE)FreeLibrary, me32.modBaseAddr, 0, nullptr);
            if (hThread) {
                WaitForSingleObject(hThread, INFINITE);
                CloseHandle(hThread);
            }
            break;
        }
        bModule = Module32NextW(hModuleSnap, &me32);
    }

    CloseHandle(hModuleSnap);
    CloseHandle(hProcess);
    return true;
}


// Utility function to convert a wide string to a lower-case wide string
std::wstring toLower(const std::wstring& input) {
    std::wstring result = input;
    std::transform(result.begin(), result.end(), result.begin(), ::tolower);
    return result;
}

// Check if a DLL is already loaded in a process
bool isDLLLoaded(DWORD processID, const std::wstring& dllName) {
    HANDLE hSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPMODULE | TH32CS_SNAPMODULE32, processID);
    if (hSnapshot != INVALID_HANDLE_VALUE) {
        MODULEENTRY32W me32;
        me32.dwSize = sizeof(MODULEENTRY32W);
        if (Module32FirstW(hSnapshot, &me32)) {
            do {
                if (toLower(me32.szModule) == toLower(dllName)) {
                    CloseHandle(hSnapshot);
                    return true;
                }
            } while (Module32NextW(hSnapshot, &me32));
        }
        CloseHandle(hSnapshot);
    }
    return false;
}

// Function to monitor and handle new and existing Darkages.exe processes
void monitorProcesses() {
    std::wstring targetProcessName = L"darkages.exe";

    while (true) {
        HANDLE hProcessSnap = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
        if (hProcessSnap == INVALID_HANDLE_VALUE) {
            continue;
        }

        PROCESSENTRY32W pe32;
        pe32.dwSize = sizeof(PROCESSENTRY32W);
        if (!Process32FirstW(hProcessSnap, &pe32)) {
            CloseHandle(hProcessSnap);
            continue;
        }

        do {
            if (toLower(pe32.szExeFile) == targetProcessName) {
                handleProcess(pe32.th32ProcessID);
            }
        } while (Process32NextW(hProcessSnap, &pe32));

        CloseHandle(hProcessSnap);
        Sleep(3000); // Wait for 10 seconds before scanning again
    }
}

void handleProcess(DWORD processID) {
    if (!isDLLLoaded(processID, L"lua.dll")) {
        InjectDLL(processID, dllPath1);
    }
    if (!isDLLLoaded(processID, L"pop.dll")) {
        InjectDLL(processID, dllPath2);
    }
}

int main(int argc, char* argv[]) {
    if (argc != 3) {
        std::cerr << "Usage: " << argv[0] << " <dllPath1> <dllPath2>" << std::endl;
        return 1;
    }

    dllPath1 = std::wstring(argv[1], argv[1] + strlen(argv[1]));
    dllPath2 = std::wstring(argv[2], argv[2] + strlen(argv[2]));

    monitorProcesses();

    return 0;
}


