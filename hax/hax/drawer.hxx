#pragma once
#include <d2d1.h>
namespace Drawer {
	void check_s_ok(HRESULT result, const char* func);
	ID2D1Factory* init_Factory();
	ID2D1HwndRenderTarget* init_RenderTarget(ID2D1Factory* pD2DFactory, HWND hWindow, int w, int h);
	ID2D1SolidColorBrush* init_SolidColorBrush(ID2D1HwndRenderTarget* pRT, D2D1::ColorF D2D1Color);

    template <class T> void SafeRelease(T** ppT)
    {
        if (*ppT)
        {
            (*ppT)->Release();
            *ppT = NULL;
        }
    }
}