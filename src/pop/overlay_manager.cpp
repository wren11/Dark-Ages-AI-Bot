// ReSharper disable CppClangTidyMiscUseAnonymousNamespace
// ReSharper disable CppFunctionResultShouldBeUsed
// ReSharper disable CppClangTidyModernizeMacroToEnum
// ReSharper disable CppClangTidyBugproneEmptyCatch
// ReSharper disable CppClangTidyBugproneNarrowingConversions
// ReSharper disable CppClangTidyClangDiagnosticImplicitFloatConversion
// ReSharper disable CppClangTidyClangDiagnosticImplicitIntFloatConversion
#include "pch.h"
#include "overlay_manager.h"
#include "io.h"
#include "spell.h"
#include "structures.h"
#include "gamestate_manager.h"
#include "script_manager.h"
#include "ui_manager.h"

static HWND g_da_hwnd;

static auto is_window_visible_and_active(const HWND h_wnd) -> bool
{
	const bool is_visible = IsWindowVisible(h_wnd) != FALSE;
	const bool is_active = GetForegroundWindow() == h_wnd;

	return is_visible && is_active;
}

HWND OverlayManager::find_game_window()
{
	return FindWindow(nullptr, L"Darkages");
}

BOOL CALLBACK OverlayManager::EnumWindowsProc(HWND hwnd, const LPARAM lParam)
{
	DWORD lpdw_process_id;
	GetWindowThreadProcessId(hwnd, &lpdw_process_id);

	if (lpdw_process_id == lParam)
	{
		SetLastError(reinterpret_cast<DWORD_PTR>(hwnd));
		return FALSE;
	}
	return TRUE;
}

void OverlayManager::position_overlay_window(const HWND hwndOverlay, const RECT &gameRect)
{
	SetWindowPos(hwndOverlay, HWND_TOPMOST, gameRect.left, gameRect.top, 1230, 615, SWP_NOACTIVATE);
}

OverlayManager::OverlayManager() : hwnd(nullptr), pD2DFactory(nullptr), pRenderTarget(nullptr)
{
}

OverlayManager::~OverlayManager()
{
	cleanup();
}

HWND OverlayManager::initialize()
{
	create_transparent_window();
	if (hwnd == nullptr)
		return nullptr;

	initialize_direct_2d();

	ui_manager.Initialize(reinterpret_cast<HINSTANCE>(GetWindowLongPtr(g_da_hwnd, GWLP_HINSTANCE)));


	return pRenderTarget != nullptr ? g_da_hwnd : nullptr;
}

void OverlayManager::create_transparent_window()
{
	const HWND h_game_wnd = find_game_window();
	if (h_game_wnd == nullptr)
		return;

	RECT game_rect;
	GetWindowRect(h_game_wnd, &game_rect);

	constexpr wchar_t class_name[] = L"OverlayWindowClass";
	WNDCLASS wc = {};
	wc.lpfnWndProc = WindowProc;
	wc.hInstance = GetModuleHandle(nullptr);
	wc.lpszClassName = class_name;
	RegisterClass(&wc);

	hwnd = CreateWindowEx(
		WS_EX_LAYERED | WS_EX_TOPMOST | WS_EX_TRANSPARENT,
		class_name,
		L"The Desi Window",
		WS_POPUP,
		game_rect.left, game_rect.top, 1240, 600,
		nullptr, nullptr, wc.hInstance, nullptr);

	SetLayeredWindowAttributes(hwnd, RGB(0, 0, 0), 0, LWA_COLORKEY);

	ShowWindow(hwnd, SW_SHOW);
	UpdateWindow(hwnd);
	SetTimer(hwnd, 1, 100, nullptr);
}

bool OverlayManager::initialize_aerial_font(HRESULT &hr, bool &value1)
{
	if (!arialFont)
	{
		hr = DWriteCreateFactory(DWRITE_FACTORY_TYPE_SHARED, __uuidof(IDWriteFactory),
								 reinterpret_cast<IUnknown **>(&pDWriteFactory));
		if (FAILED(hr))
		{
			value1 = true;
			return true;
		}
		hr = pDWriteFactory->CreateTextFormat(
			L"Verdana",
			nullptr,
			DWRITE_FONT_WEIGHT_NORMAL,
			DWRITE_FONT_STYLE_NORMAL,
			DWRITE_FONT_STRETCH_NORMAL,
			18.0f,
			L"en-us",
			&arialFont);

		if (FAILED(hr))
		{
			value1 = true;
			return true;
		}
	}
	return false;
}

bool OverlayManager::initialize_timer_font(HRESULT &hr, bool &value1)
{
	if (!timerFont)
	{
		hr = pDWriteFactory->CreateTextFormat(
			L"Arial",
			nullptr,
			DWRITE_FONT_WEIGHT_NORMAL,
			DWRITE_FONT_STYLE_NORMAL,
			DWRITE_FONT_STRETCH_NORMAL,
			20.0f,
			L"en-us",
			&timerFont);

		if (FAILED(hr))
		{
			value1 = true;
			return true;
		}
	}
	return false;
}

bool OverlayManager::initialize_fonts(HRESULT &hr)
{
	bool err;

	if (initialize_aerial_font(hr, err))
		return err;

	if (initialize_timer_font(hr, err))
		return err;

	return false;
}

void OverlayManager::initialize_bitmaps()
{
	const auto filesMap = load_bmp_files_map();

	for (const auto &pair : filesMap)
	{
		ID2D1Bitmap *pBitmap = nullptr;
		pBitmap = LoadBitmapFromFile(pRenderTarget, pIWICFactory, pair.second.c_str());

		if (pBitmap != nullptr)
		{
			pBitmaps[pair.first] = pBitmap;
			std::wcout << L"Loaded: " << pair.first << L" -> " << pair.second << std::endl;
		}
		else
		{
			std::wcerr << L"Failed to load: " << pair.first << L" -> " << pair.second << std::endl;
		}
	}
}

bool OverlayManager::initialize_brushes(HRESULT &hr)
{
	hr = pRenderTarget->CreateSolidColorBrush(D2D1::ColorF(D2D1::ColorF::Green), &greenBrush);
	if (FAILED(hr))
	{
		return true;
	}

	hr = pRenderTarget->CreateSolidColorBrush(D2D1::ColorF(D2D1::ColorF::Black), &blackBrush);
	if (FAILED(hr))
	{
		return true;
	}

	hr = pRenderTarget->CreateSolidColorBrush(D2D1::ColorF(D2D1::ColorF::White), &whiteBrush);
	if (FAILED(hr))
	{
		return true;
	}

	hr = pRenderTarget->CreateSolidColorBrush(D2D1::ColorF(D2D1::ColorF::Red), &redBrush);
	if (FAILED(hr))
	{
		return true;
	}

	hr = pRenderTarget->CreateSolidColorBrush(D2D1::ColorF(D2D1::ColorF::Yellow), &yellowBrush);
	if (FAILED(hr))
	{
		return true;
	}

	hr = pRenderTarget->CreateSolidColorBrush(D2D1::ColorF(D2D1::ColorF::Orange), &orangeBrush);
	if (FAILED(hr))
	{
		return true;
	}

	hr = pRenderTarget->CreateSolidColorBrush(D2D1::ColorF(D2D1::ColorF::Aqua), &aruaBrush);
	if (FAILED(hr))
	{
		return true;
	}
	return false;
}

void OverlayManager::initialize_direct_2d()
{
	HRESULT hr = D2D1CreateFactory(
		D2D1_FACTORY_TYPE_SINGLE_THREADED,
		&pD2DFactory);

	if (FAILED(hr))
		return;

	D2D1CreateFactory(
		D2D1_FACTORY_TYPE_SINGLE_THREADED,
		&pD2DFactory);

	CoCreateInstance(
		CLSID_WICImagingFactory,
		NULL,
		CLSCTX_INPROC_SERVER,
		IID_IWICImagingFactory,
		reinterpret_cast<void **>(&pIWICFactory));

	RECT rc;
	GetClientRect(hwnd, &rc);

	const D2D1_SIZE_U size = D2D1::SizeU(rc.right - rc.left, rc.bottom - rc.top);

	hr = pD2DFactory->CreateHwndRenderTarget(
		D2D1::RenderTargetProperties(),
		D2D1::HwndRenderTargetProperties(hwnd, size),
		&pRenderTarget);

	if (FAILED(hr))
		return;

	initialize_bitmaps();

	if (initialize_brushes(hr))
		return;

	if (initialize_fonts(hr))
		return;
}

D2D1_SIZE_F OverlayManager::measure_string(
	IDWriteFactory *pDWriteFactory,
	ID2D1RenderTarget *pRenderTarget,
	const std::wstring &text,
	IDWriteTextFormat *textFormat,
	const D2D1_RECT_F &layoutRect)
{
	D2D1_SIZE_F text_size = D2D1::SizeF(0.0f, 0.0f);

	if (!pDWriteFactory || !pRenderTarget || text.empty() || !textFormat)
	{
		return text_size;
	}

	IDWriteTextLayout *p_text_layout = nullptr;

	const HRESULT hr = pDWriteFactory->CreateTextLayout(
		text.c_str(),
		static_cast<UINT32>(text.length()),
		textFormat,
		layoutRect.right - layoutRect.left,
		layoutRect.bottom - layoutRect.top,
		&p_text_layout);

	if (FAILED(hr) || !p_text_layout)
		return text_size;

	DWRITE_TEXT_METRICS text_metrics;

	p_text_layout->GetMetrics(&text_metrics);

	text_size.width = text_metrics.width;
	text_size.height = text_metrics.height;

	p_text_layout->Release();

	return text_size;
}

constexpr int tile_width = 56;	// The width of a tile
constexpr int tile_height = 28; // The height of a tile

static std::pair<int, int> tile_to_screen_position(
	const int offsetX,
	const int offsetY,
	const int screenWidth,
	const int screenHeight)
{
	int screen_x = ((offsetX) - (offsetY)) * (tile_width) + (screenWidth / 2);
	int screen_y = (offsetY + offsetX) * (tile_height) + (screenHeight / 2);

	screen_y += tile_height * 3;

	return std::make_pair(screen_x, screen_y);
}

std::string toLower(const std::string &str)
{
	std::string lowerStr = str;
	std::transform(lowerStr.begin(), lowerStr.end(), lowerStr.begin(),
				   [](unsigned char c)
				   { return std::tolower(c); });
	return lowerStr;
}

bool isHostile(const std::vector<std::string> &hostile_players, const std::string &name)
{
	const std::string lowerName = toLower(name);
	for (const auto &player : hostile_players)
	{
		if (toLower(player) == lowerName)
		{
			return true;
		}
	}
	return false;
}

void OverlayManager::draw_players()
{
	const Location playerLocation = game_state.get_player_location();

	game_state.player_manager.ForEach([&](const std::shared_ptr<Player> &objectPtr)
									  {
			if (!objectPtr)
				return;

			const Player& object = *objectPtr;

			const int offsetX = object.GetLocationX() - playerLocation.X;
			const int offsetY = object.GetLocationY() - playerLocation.Y;

			auto [screenX, screenY] = tile_to_screen_position(offsetX, offsetY, 1230, 615);

			screenX += 10;
			screenY += 20;

			if (object.HasSeal())
			{
				draw_bitmap_at_position(
					pRenderTarget,
					pBitmaps[_T("demise")],
					screenX - 10,
					screenY - 10
				);
			}

			if (isHostile(game_state.hostile_players, toLower(object.GetName())))
			{
				DrawCenteredText(
					object.GetName(),
					screenX,
					screenY - (tile_height * 4.1),
					redBrush,
					arialFont
				);
			}
			else
			{
				DrawCenteredText(
					object.GetName(),
					screenX,
					screenY - (tile_height * 4.1),
					whiteBrush,
					arialFont
				);
			} });
}

void OverlayManager::draw_animations()
{
	const Location playerLocation = game_state.get_player_location();

	game_state.animations_manager.ForEach([&](int targetId, const AnimationTiming &timing)
										  { game_state.player_manager.GetAndApplyAction(
												targetId,
												[playerLocation, timing, this](const Player *player)
												{
													const int offset_x = player->GetLocationX() - playerLocation.X;
													const int offset_y = player->GetLocationY() - playerLocation.Y;

													auto [screenX, screenY] = tile_to_screen_position(offset_x, offset_y, 1230, 615);

													const std::string longTimerText = std::to_string(static_cast<int>(timing.getLongTimer()));
													const std::string shortTimerText = std::to_string(static_cast<int>(timing.getShortTimer()));

													const int timerOffsetY = screenY - 50;

													if (timing.getLongTimer() > 0)
													{
														DrawCenteredText(
															longTimerText,
															screenX + 20,
															timerOffsetY,
															aruaBrush,
															timerFont);
													}

													if (timing.getShortTimer() > 0)
													{
														DrawCenteredText(
															shortTimerText,
															screenX + 20,
															timerOffsetY - 25,
															yellowBrush,
															timerFont);
													}
												}); });
}

void OverlayManager::draw_sprites()
{
}

void OverlayManager::draw_animation_timers()
{
}

void OverlayManager::DrawOverlay()
{
	try
	{
		if (!pRenderTarget || !arialFont || !whiteBrush)
			return;

		if (g_da_hwnd != nullptr && !is_window_visible_and_active(g_da_hwnd))
		{
			pRenderTarget->BeginDraw();
			pRenderTarget->Clear(D2D1::ColorF(D2D1::ColorF::Black, 0.0f));
			pRenderTarget->EndDraw();
			return;
		}

		pRenderTarget->BeginDraw();
		pRenderTarget->Clear(D2D1::ColorF(D2D1::ColorF::Black, 0.0f));

		draw_players();
		draw_animations();
		draw_sprites();
		draw_animation_timers();

		if (const HRESULT hr = pRenderTarget->EndDraw(); hr == D2DERR_RECREATE_TARGET)
		{
			cleanup();
			initialize_direct_2d();
		}
	}
	catch (const std::exception &e)
	{
	}
	catch (...)
	{
	}
}

void OverlayManager::run()
{
	MSG msg = {};

	while (GetMessage(&msg, nullptr, 0, 0))
	{
		try
		{
			if (msg.message == WM_SETCURSOR)
				return;

			TranslateMessage(&msg);
			DispatchMessage(&msg);

			DrawOverlay();
		}
		catch (...)
		{
		}
	}
}

void OverlayManager::cleanup()
{
	if (pRenderTarget)
	{
		pRenderTarget->Release();
		pRenderTarget = nullptr;
	}

	if (pD2DFactory)
	{
		pD2DFactory->Release();
		pD2DFactory = nullptr;
	}

	if (hwnd)
	{
		DestroyWindow(hwnd);
		hwnd = nullptr;
	}

	ui_manager.Cleanup();
	g_da_hwnd = nullptr;
	

}

LRESULT CALLBACK OverlayManager::WindowProc(
	const HWND lp_hwnd,
	const UINT u_msg,
	const WPARAM w_param,
	const LPARAM l_param)
{
	switch (u_msg)
	{
	case WM_TIMER:
	{
		if (const HWND h_game_wnd = find_game_window())
		{
			g_da_hwnd = h_game_wnd;

			RECT gameRect;
			GetWindowRect(h_game_wnd, &gameRect);
			position_overlay_window(lp_hwnd, gameRect);
		}
		break;
	}
	case WM_DESTROY:
		PostQuitMessage(0);
		break;
	default:
		return DefWindowProc(
			lp_hwnd,
			u_msg,
			w_param,
			l_param);
	}
	return 0;
}

OverlayManager drawing_manager;
