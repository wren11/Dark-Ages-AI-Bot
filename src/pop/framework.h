#pragma once
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <ddraw.h>
#include <objidl.h>
#include <gdiplus.h>

#include <detours.h>
#pragma comment(lib, "detours.lib")

using namespace Gdiplus;
#pragma comment(lib, "Gdiplus.lib")

#include <DbgHelp.h>
#pragma comment(lib, "Dbghelp.lib")

#include <d2d1.h>
#pragma comment(lib, "d2d1.lib")

#include <dwrite.h>
#include <wincodec.h>
#pragma comment(lib, "dwrite.lib")

#pragma comment(lib, "windowscodecs.lib")
#include <commdlg.h>