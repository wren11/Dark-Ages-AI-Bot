#pragma once
#include "pch.h"
#include "gamestate_manager.h"
#include "script_manager.h"

#define ID_LOAD_SCRIPT 1
#define ID_START_SCRIPT 2
#define ID_STOP_SCRIPT 3

class GuiManager {
public:
    inline static HWND hGuiWnd = nullptr;
    inline static HHOOK hKeyboardHook = nullptr;
    inline static bool guiVisible = false;

    inline static HWND hScriptDisplay = nullptr;
    inline static HWND hStartButton = nullptr;
    inline static HWND hStopButton = nullptr;

    static void Initialize(HINSTANCE hInstance) {
        std::thread initThread(&GuiManager::InitThreadProc, hInstance);
        initThread.detach();
    }

    static void Cleanup() {
        RemoveKeyboardHook();
        if (hGuiWnd != nullptr) {
            DestroyWindow(hGuiWnd);
        }
    }

private:
    static void InitThreadProc(HINSTANCE hInstance) {
        SetKeyboardHook();
        CreateGuiWindow(hInstance);

        MSG msg;
        while (GetMessage(&msg, nullptr, 0, 0)) {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
    }

    static void ToggleGuiVisibility() {
        guiVisible = !guiVisible;
        ShowWindow(hGuiWnd, guiVisible ? SW_SHOW : SW_HIDE);
    }

    static LRESULT CALLBACK KeyboardProc(int nCode, WPARAM wParam, LPARAM lParam) {
        if (nCode == HC_ACTION) {
            PKBDLLHOOKSTRUCT p = (PKBDLLHOOKSTRUCT)lParam;
            if (wParam == WM_KEYDOWN && p->vkCode == VK_F12) {
                ToggleGuiVisibility();
            }
        }
        return CallNextHookEx(hKeyboardHook, nCode, wParam, lParam);
    }

    static void SetKeyboardHook() {
        hKeyboardHook = SetWindowsHookEx(WH_KEYBOARD_LL, KeyboardProc, NULL, 0);
    }

    static void RemoveKeyboardHook() {
        UnhookWindowsHookEx(hKeyboardHook);
    }

    static void CreateMenu(HWND hWnd) {
        HMENU hMenubar = ::CreateMenu(); 
        HMENU hMenu = ::CreatePopupMenu(); 

        AppendMenu(hMenu, MF_STRING, ID_LOAD_SCRIPT, L"Load Script");
        AppendMenu(hMenubar, MF_POPUP, (UINT_PTR)hMenu, L"File");

        SetMenu(hWnd, hMenubar);
    }

    static void CreateScriptDisplayControl(HWND hWnd) {
        hScriptDisplay = CreateWindowEx(WS_EX_CLIENTEDGE, L"EDIT", L"",
            WS_CHILD | WS_VISIBLE | WS_VSCROLL | ES_MULTILINE | ES_AUTOVSCROLL,
            10, 10, 300, 200, hWnd, nullptr, GetModuleHandle(nullptr), nullptr);
    }

    static void CreateControlButtons(HWND hWnd) {
        hStartButton = CreateWindow(L"BUTTON", L"Start", WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_DEFPUSHBUTTON,
            10, 220, 100, 24, hWnd, (HMENU)ID_START_SCRIPT, GetModuleHandle(nullptr), nullptr);
        hStopButton = CreateWindow(L"BUTTON", L"Stop", WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_DEFPUSHBUTTON,
            120, 220, 100, 24, hWnd, (HMENU)ID_STOP_SCRIPT, GetModuleHandle(nullptr), nullptr);
    }

    static std::string WideStringToString(const std::wstring& wstr) {
        if (wstr.empty()) return std::string();
        int sizeNeeded = WideCharToMultiByte(CP_UTF8, 0, &wstr[0], (int)wstr.size(), NULL, 0, NULL, NULL);
        std::string strTo(sizeNeeded, 0);
        WideCharToMultiByte(CP_UTF8, 0, &wstr[0], (int)wstr.size(), &strTo[0], sizeNeeded, NULL, NULL);
        return strTo;
    }

    static void LoadLuaScript(const std::wstring& filePath) {
        std::string narrowFilePath = WideStringToString(filePath);
        script_manager.LoadScript(narrowFilePath);
    }

    static void StartLuaScript() {
        script_manager.StartScript();
    }

    static void StopLuaScript() {
        script_manager.StopScript();
    }

    static LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam) {
        switch (message) {
        case WM_COMMAND: {
            switch (LOWORD(wParam)) {
            case ID_LOAD_SCRIPT: {
                WCHAR fileName[MAX_PATH] = L"";
                OPENFILENAME ofn = { 0 };
                ofn.lStructSize = sizeof(ofn);
                ofn.hwndOwner = hWnd;
                ofn.lpstrFile = fileName;
                ofn.nMaxFile = MAX_PATH;
                ofn.lpstrFilter = L"Lua Scripts\0*.lua\0All Files\0*.*\0";
                ofn.Flags = OFN_EXPLORER | OFN_FILEMUSTEXIST | OFN_HIDEREADONLY;
                if (GetOpenFileName(&ofn)) {
                    LoadLuaScript(fileName);
                }
                break;
            }
            case ID_START_SCRIPT:
                StartLuaScript();
                break;
            case ID_STOP_SCRIPT:
                StopLuaScript();
                break;
            }
            break;
        }
        case WM_SIZE:
            // Handle window resizing
            break;
        case WM_DESTROY:
            PostQuitMessage(0);
            break;
        default:
            return DefWindowProc(hWnd, message, wParam, lParam);
        }
        return 0;
    }

    static void CreateGuiWindow(HINSTANCE hInstance) {
        WNDCLASSEX wcex = { 0 };
        wcex.cbSize = sizeof(WNDCLASSEX);
        wcex.style = CS_HREDRAW | CS_VREDRAW;
        wcex.lpfnWndProc = WndProc;
        wcex.cbClsExtra = 0;
        wcex.cbWndExtra = 0;
        wcex.hInstance = hInstance;
        wcex.hIcon = LoadIcon(hInstance, IDI_APPLICATION); 
        wcex.hCursor = LoadCursor(nullptr, IDC_ARROW);
        wcex.hbrBackground = (HBRUSH)(COLOR_WINDOW+1);
        wcex.lpszMenuName = nullptr;
        wcex.lpszClassName = L"GameGuiClass";
        wcex.hIconSm = LoadIcon(wcex.hInstance, IDI_APPLICATION);

        if (!RegisterClassEx(&wcex)) {
            MessageBox(nullptr, L"Call to RegisterClassEx failed!", L"Game GUI", 0);
            return;
        }

        hGuiWnd = CreateWindow(
            wcex.lpszClassName, // Window class name
            L"Game GUI", // Window title
            WS_OVERLAPPEDWINDOW, // Window style
            CW_USEDEFAULT, CW_USEDEFAULT, // Position x, y
            500, 400, // Width, height
            nullptr, // Parent window
            nullptr, // Menu
            hInstance, // Instance handle
            nullptr // Additional application data
        );

        if (!hGuiWnd) {
            MessageBox(nullptr, L"Call to CreateWindow failed!", L"Game GUI", 0);
            return;
        }

        CreateMenu(hGuiWnd);
        CreateScriptDisplayControl(hGuiWnd);
        CreateControlButtons(hGuiWnd);

        ShowWindow(hGuiWnd, SW_HIDE);
        UpdateWindow(hGuiWnd);
    }
};

extern GuiManager ui_manager;