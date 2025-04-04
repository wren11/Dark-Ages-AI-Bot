#pragma once

// Use lean and mean Windows headers
#define WIN32_LEAN_AND_MEAN
#include <windows.h>

// Detours for function hooking (essential for the bot)
#include <detours.h>
#pragma comment(lib, "detours.lib")

// GDI+ for UI rendering if needed by the game menu
#include <gdiplus.h>
#pragma comment(lib, "Gdiplus.lib")

// DirectX for potential game rendering hooks
#include <d2d1.h>
#pragma comment(lib, "d2d1.lib")

// DirectWrite for text rendering in UI
#include <dwrite.h>
#pragma comment(lib, "dwrite.lib")