#include <iostream>
#include "drawer.hxx"

using namespace std;

void Drawer::check_s_ok(HRESULT result, const char* func)
{
    if (FAILED(result))
    {
        cout << "Error : " << func << " (...) failed." << endl;
        exit(-1);
    }
}

ID2D1Factory* Drawer::init_Factory()
{
    ID2D1Factory* pD2DFactory = NULL;
    HRESULT hr = D2D1CreateFactory(
        D2D1_FACTORY_TYPE_SINGLE_THREADED,
        &pD2DFactory
        );
    check_s_ok(hr, "init_Factory");
    return (pD2DFactory);
}

ID2D1HwndRenderTarget* Drawer::init_RenderTarget(ID2D1Factory* pD2DFactory, HWND hWindow, int w, int h)
{
    ID2D1HwndRenderTarget* pRT = NULL;
    HRESULT hr = pD2DFactory->CreateHwndRenderTarget(
        D2D1::RenderTargetProperties(
            D2D1_RENDER_TARGET_TYPE_DEFAULT,
            D2D1::PixelFormat(
                DXGI_FORMAT_UNKNOWN,
                D2D1_ALPHA_MODE_PREMULTIPLIED
                )
            ),
        D2D1::HwndRenderTargetProperties(
            hWindow,
            D2D1::SizeU(w, h)
            ),
        &pRT
        );
    check_s_ok(hr, "init_RenderTarget");
    return (pRT);
}

ID2D1SolidColorBrush* Drawer::init_SolidColorBrush(ID2D1HwndRenderTarget* pRT, D2D1::ColorF D2D1Color)
{
    // Create brush
    ID2D1SolidColorBrush* pColorBrush = NULL;
    HRESULT hr = pRT->CreateSolidColorBrush(
        D2D1::ColorF(D2D1Color),
        &pColorBrush
        );
    check_s_ok(hr, "init_SolidColorBrush");
    return (pColorBrush);
}
