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


void Drawer::DrawBox(ID2D1HwndRenderTarget* pRT, ID2D1SolidColorBrush* pCB, Entity pEntity)
{
    pCB->SetColor(D2D1::ColorF(D2D1::ColorF::Red));
    pRT->DrawRectangle(
        D2D1::RectF(
            pEntity.sEntity.Head.x,
            pEntity.sEntity.Foot.y,
            pEntity.sEntity.Head.x + pEntity.sEntity.RectWidth,
            pEntity.sEntity.Head.y),
        pCB);
}

void Drawer::DrawLife(ID2D1HwndRenderTarget* pRT, ID2D1SolidColorBrush* pCB, Entity pEntity)
{
    float HealthBar = pEntity.sEntity.Health / 100.f;
    // color calculus
    float r, g, b = 0.f;
    r = 1 - (1 * HealthBar);
    g = 1 * HealthBar;
    pCB->SetColor(D2D1::ColorF(D2D1::ColorF(r, g, b)));
    // end color calculus

    pRT->FillRectangle(
        D2D1::RectF(
            pEntity.sEntity.Head.x,
            pEntity.sEntity.Head.y,
            pEntity.sEntity.Head.x + (HealthBar * pEntity.sEntity.RectWidth),
            pEntity.sEntity.Head.y - 4.f),
        pCB);
}

void Drawer::DrawArmor(ID2D1HwndRenderTarget* pRT, ID2D1SolidColorBrush* pCB, Entity pEntity)
{
    float ArmorBar = pEntity.sEntity.Armor / 100.f;
    pCB->SetColor(D2D1::ColorF(D2D1::ColorF(0.26, 0.52, 0.98)));
    pEntity.sEntity.Head.y -= 4.f;
    pRT->FillRectangle(
        D2D1::RectF(
            pEntity.sEntity.Head.x,
            pEntity.sEntity.Head.y,
            pEntity.sEntity.Head.x + (ArmorBar * pEntity.sEntity.RectWidth),
            pEntity.sEntity.Head.y - 4.f),
        pCB);
}