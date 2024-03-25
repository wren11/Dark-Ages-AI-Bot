#pragma once
#include "pch.h"
#include "gamestate_manager.h"

class OverlayManager
{
public:
	OverlayManager();
	~OverlayManager();

	HWND initialize();
	void initialize_bitmaps();
	void initialize_direct_2d();
	void run();
	void cleanup();

private:
	HWND hwnd; // Handle to the overlay window
	ID2D1SolidColorBrush *greenBrush = nullptr;
	ID2D1SolidColorBrush *blackBrush = nullptr;
	ID2D1SolidColorBrush *whiteBrush = nullptr;
	ID2D1SolidColorBrush *redBrush = nullptr;
	ID2D1SolidColorBrush *orangeBrush = nullptr;
	ID2D1SolidColorBrush *yellowBrush = nullptr;
	ID2D1SolidColorBrush *aruaBrush = nullptr;
	ID2D1Bitmap *kelb = nullptr;
	IDWriteFactory *pDWriteFactory = nullptr;
	IWICImagingFactory *pIWICFactory = nullptr;
	IDWriteTextFormat *arialFont = nullptr;
	IDWriteTextFormat *timerFont = nullptr;

	ID2D1Factory *pD2DFactory = nullptr;
	ID2D1HwndRenderTarget *pRenderTarget = nullptr;

	void create_transparent_window();
	bool initialize_aerial_font(HRESULT &hr, bool &value1);
	bool initialize_timer_font(HRESULT &hr, bool &value1);
	bool initialize_fonts(HRESULT &hr);
	bool initialize_brushes(HRESULT &hr);
	void draw_players();
	void draw_animations();
	static void draw_sprites();
	static void draw_animation_timers();
	void DrawOverlay();
	static LRESULT CALLBACK WindowProc(HWND lp_hwnd, UINT u_msg, WPARAM w_param, LPARAM l_param);
	static HWND find_game_window();
	static void position_overlay_window(HWND hwndOverlay, const RECT &gameRect);
	static BOOL CALLBACK EnumWindowsProc(HWND hwnd, LPARAM lParam);

	static D2D1_SIZE_F measure_string(IDWriteFactory *pDWriteFactory, ID2D1RenderTarget *pRenderTarget,
									  const std::wstring &text, IDWriteTextFormat *textFormat, const D2D1_RECT_F &layoutRect);

	std::unordered_map<std::wstring, ID2D1Bitmap *> pBitmaps;

	ID2D1Bitmap *LoadBitmapFromFile(
		ID2D1RenderTarget *pRenderTarget,
		IWICImagingFactory *pIWICFactory,
		PCWSTR uri)
	{
		IWICBitmapDecoder *pDecoder = NULL;
		IWICBitmapFrameDecode *pSource = NULL;
		IWICFormatConverter *pConverter = NULL;
		ID2D1Bitmap *pBitmap = NULL;

		// Create a decoder
		HRESULT hr = pIWICFactory->CreateDecoderFromFilename(
			uri,
			NULL,
			GENERIC_READ,
			WICDecodeMetadataCacheOnLoad,
			&pDecoder);

		if (SUCCEEDED(hr))
		{
			// Get the first frame of the image
			hr = pDecoder->GetFrame(0, &pSource);
		}
		if (SUCCEEDED(hr))
		{
			// Convert the image format to 32bppPBGRA
			hr = pIWICFactory->CreateFormatConverter(&pConverter);
		}
		if (SUCCEEDED(hr))
		{
			hr = pConverter->Initialize(
				pSource,
				GUID_WICPixelFormat32bppPBGRA,
				WICBitmapDitherTypeNone,
				NULL,
				0.f,
				WICBitmapPaletteTypeMedianCut);
		}
		if (SUCCEEDED(hr))
		{
			// Create a Direct2D bitmap from the WIC bitmap
			hr = pRenderTarget->CreateBitmapFromWicBitmap(
				pConverter,
				NULL,
				&pBitmap);
		}

		if (pDecoder)
			pDecoder->Release();
		if (pSource)
			pSource->Release();
		if (pConverter)
			pConverter->Release();

		return pBitmap;
	}

	auto draw_bitmap_at_position(ID2D1RenderTarget *pRenderTarget, ID2D1Bitmap *pBitmap, float screenX, float screenY) -> void
	{
		D2D1_SIZE_F size = pBitmap->GetSize();
		float drawY = screenY - size.height;

		D2D1_RECT_F rect = D2D1::RectF(screenX, drawY, screenX + size.width, screenY);
		pRenderTarget->DrawBitmap(pBitmap, rect);
	}

	ID2D1PathGeometry *CreatePolygonGeometry(const D2D1_POINT_2F *points, UINT32 count) const
	{
		if (!pD2DFactory)
		{
			return nullptr;
		}

		ID2D1PathGeometry *pPathGeometry = nullptr;
		pD2DFactory->CreatePathGeometry(&pPathGeometry);

		if (pPathGeometry)
		{
			ID2D1GeometrySink *pSink = nullptr;
			pPathGeometry->Open(&pSink);

			if (pSink)
			{
				pSink->BeginFigure(points[0], D2D1_FIGURE_BEGIN_FILLED);
				pSink->AddLines(&points[1], count - 1);
				pSink->EndFigure(D2D1_FIGURE_END_CLOSED);
				pSink->Close();
				pSink->Release();
			}
		}

		return pPathGeometry;
	}

	void DrawPolygon(const std::vector<D2D1_POINT_2F> &points, ID2D1Brush *brush, FLOAT strokeWidth) const
	{
		if (!pRenderTarget || points.size() < 2)
		{
			return;
		}

		ID2D1PathGeometry *pPathGeometry = nullptr;
		HRESULT hr = pD2DFactory->CreatePathGeometry(&pPathGeometry);
		if (FAILED(hr))
		{
			// Handle error creating path geometry
			return;
		}

		ID2D1GeometrySink *pSink = nullptr;
		hr = pPathGeometry->Open(&pSink);
		if (FAILED(hr))
		{
			// Handle error opening geometry sink
			pPathGeometry->Release();
			return;
		}

		pSink->BeginFigure(points[0], D2D1_FIGURE_BEGIN_FILLED);
		for (size_t i = 1; i < points.size(); ++i)
		{
			pSink->AddLine(points[i]);
		}
		pSink->EndFigure(D2D1_FIGURE_END_CLOSED);
		hr = pSink->Close();
		if (FAILED(hr))
		{
			// Handle error closing geometry sink
			pSink->Release();
			pPathGeometry->Release();
			return;
		}
		pSink->Release();

		pRenderTarget->DrawGeometry(pPathGeometry, brush, strokeWidth);

		pPathGeometry->Release();
	}

	void DrawText(const wchar_t *text, const D2D1_RECT_F &layoutRect, ID2D1Brush *brush,
				  IDWriteTextFormat *textFormat) const
	{
		if (!pRenderTarget || !text || !brush || !textFormat)
		{
			return;
		}

		pRenderTarget->DrawText(
			text,
			wcslen(text),
			textFormat,
			layoutRect,
			brush);
	}

	void DrawText(const std::string &text, FLOAT posX, FLOAT posY)
	{
		if (!pRenderTarget || !arialFont || !whiteBrush)
		{
			return; // Ensure render target, font, and brush are initialized
		}

		// Convert std::string to std::wstring
		std::wstring wideText(text.begin(), text.end());

		// Calculate the layout rectangle based on the screen position
		D2D1_RECT_F layoutRect = D2D1::RectF(posX, posY, posX + 200.0f, posY + 50.0f);
		// Adjust width and height as needed

		// Draw the text
		pRenderTarget->DrawText(
			wideText.c_str(),
			wideText.length(),
			arialFont,
			layoutRect,
			whiteBrush);
	}

	void DrawCenteredText(const std::string &text, FLOAT centerX, FLOAT centerY, ID2D1Brush *brush, IDWriteTextFormat *font)
	{
		if (!pRenderTarget || !pDWriteFactory || !arialFont || !whiteBrush)
		{
			return;
		}

		std::wstring wideText(text.begin(), text.end());

		// Assuming a very large layout rectangle to measure the text without constraints
		D2D1_RECT_F layoutRect = D2D1::RectF(0, 0, 10000, 10000); // Large enough to not constrain the text

		// Calculate the size of the text within the layoutRect
		D2D1_SIZE_F textSize = measure_string(pDWriteFactory, pRenderTarget, wideText, arialFont, layoutRect);

		// Calculate the top-left corner for the text to center it
		FLOAT left = centerX - textSize.width / 2.0f;
		FLOAT top = centerY - textSize.height / 2.0f;

		// Define the rectangle where the text should be drawn
		D2D1_RECT_F textRect = D2D1::RectF(left, top, left + textSize.width, top + textSize.height);

		// Draw the text
		pRenderTarget->DrawText(
			wideText.c_str(),
			static_cast<UINT32>(text.length()),
			font,
			&textRect,
			brush);
	}
};

extern OverlayManager drawing_manager;
