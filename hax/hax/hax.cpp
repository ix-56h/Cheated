#include <dwmapi.h>
#include <tlhelp32.h>
#include <thread>
#include "hax.hxx"
#include "drawer.hxx"
#include "Entities.hxx"
#include "Reader.hxx"

using namespace std;

DWORD   pID         = NULL;
DWORD   baseAddr    = NULL;
bool    stop        = true;
int     width       = 0;
int     height      = 0;
POINT   xy;

HRESULT get_pid(const char *pName)
{
    PROCESSENTRY32 pentry;
    HANDLE snapshot = NULL;
    // Create snapshot of all processes
    snapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
    
    if (snapshot == INVALID_HANDLE_VALUE)
    {
        cout << "Oh fuck, failed to get snapshot" << endl;
        CloseHandle(snapshot);
        return (ERROR_SUCCESS);
    }
    // Init the struct size
    pentry.dwSize = sizeof(PROCESSENTRY32);
    // Check if we have a first element in processes snapshot
    if (Process32First(snapshot, &pentry))
    {
        // iterate on it until we get the process
        while (Process32Next(snapshot, &pentry))
        {
            if (!strcmp(pName, pentry.szExeFile))
            {
                cout << "We have te process id : " << pentry.th32ProcessID << endl;
                // save pID
                pID = pentry.th32ProcessID;
                break;
            }
        }
    }

    CloseHandle(snapshot);
    if (pID == NULL)
        return (ERROR_SUCCESS);
    return (S_OK);
}

HRESULT get_baseaddr(DWORD pid, const char* mName)
{
    MODULEENTRY32 mentry;
    HANDLE snapshot = NULL;
    // Create a snapshot of all modules of the process associated to the pID
    snapshot = CreateToolhelp32Snapshot(TH32CS_SNAPMODULE, pID);

    if (snapshot == INVALID_HANDLE_VALUE)
    {
        cout << "Oh fuck, failed to get snapshot" << endl;
        CloseHandle(snapshot);
        return (ERROR_SUCCESS);
    }

    // Init the struct size
    mentry.dwSize = sizeof(MODULEENTRY32);
    // Check if we have a first element in modules snapshot
    if (Module32First(snapshot, &mentry))
    {
        // iterate on it until we get the module
        while (Module32Next(snapshot, &mentry))
        {
            if (!strcmp(mName, mentry.szModule))
            {
                cout << "Base address of " << mentry.szModule << " is " << hex << (unsigned int)mentry.modBaseAddr << endl;
                // save the base address
                baseAddr = (unsigned int)mentry.modBaseAddr;
                break;
            }
        }
    }
    CloseHandle(snapshot);
    if (baseAddr == NULL)
        return (ERROR_SUCCESS);
    return (S_OK);
}

struct vector WorldToScreen(const struct vector pos, struct view_matrix_t matrix) {
    struct vector out;
    float _x = matrix.matrix[0] * pos.x + matrix.matrix[1] * pos.y + matrix.matrix[2] * pos.z + matrix.matrix[3];
    float _y = matrix.matrix[4] * pos.x + matrix.matrix[5] * pos.y + matrix.matrix[6] * pos.z + matrix.matrix[7];
    out.z = matrix.matrix[12] * pos.x + matrix.matrix[13] * pos.y + matrix.matrix[14] * pos.z + matrix.matrix[15];

    _x *= 1.f / out.z;
    _y *= 1.f / out.z;
 
    out.x = width * .5f;
    out.y = height * .5f;

    out.x += 0.5f * _x * width + 0.5f;
    out.y -= 0.5f * _y * height + 0.5f;

    return out;
}

HANDLE  init_processes(const char *ProcessName, const char *ModuleName)
{
    // Get de pID of the process
    if (!SUCCEEDED(get_pid(ProcessName)))
        return (NULL);
    // Get the module base address
    if (!SUCCEEDED(get_baseaddr(pID, ModuleName)))
        return (NULL);

    // Get Handle on process
    HANDLE hProcess = OpenProcess(PROCESS_ALL_ACCESS, false, pID);
    return (hProcess);
}

inline POINT get_client_window_position(const HWND window_handle)
{
    RECT rectangle;

    GetClientRect(window_handle, static_cast<LPRECT>(&rectangle));
    MapWindowPoints(window_handle, nullptr, reinterpret_cast<LPPOINT>(&rectangle), 2);

    const POINT coordinates = { rectangle.left, rectangle.top };

    return coordinates;
}

void    WindowResizer(HWND hTargetWnd, HWND hWnd)
{
    while (stop)
    {
        POINT cur = get_client_window_position(hTargetWnd);
        if (cur.x != xy.x || cur.y != xy.y)
        {
            xy = cur;
            MoveWindow(hWnd, xy.x, xy.y, width, height, false);
        }
        Sleep(1000);
    }
}

int WINAPI wWinMain(HINSTANCE hInstance, HINSTANCE, PWSTR pCmdLine, int nCmdShow) 
{
    WNDCLASSEX wcx;
    ZeroMemory(&wcx, sizeof(WNDCLASSEX));

    wcx.cbSize = sizeof(WNDCLASSEX);
    wcx.hbrBackground = (HBRUSH)CreateSolidBrush(RGB(0, 0, 0));
    wcx.hInstance = hInstance;
    wcx.lpfnWndProc = WindowProc;
    wcx.lpszClassName = "MainWindow";
    wcx.style = CS_HREDRAW | CS_VREDRAW;

    RegisterClassEx(&wcx);

    RECT    WBounds;
    // Open a handle to the device context from the top-level handle window (here csgo window)
    HWND hTargetWnd = FindWindowA(NULL, "Counter-Strike: Global Offensive");
    if (hTargetWnd == NULL)
    {
        cout << "Error: failed to FindWindowA." << endl;
        return (0);
    }
    GetClientRect(hTargetWnd, &WBounds);
    xy = get_client_window_position(hTargetWnd);
    width = WBounds.right - WBounds.left;
    height = WBounds.bottom - WBounds.top;
    AdjustWindowRectEx(&WBounds, WS_OVERLAPPEDWINDOW, false, WS_EX_OVERLAPPEDWINDOW);
    HWND hWnd = CreateWindowEx(WS_EX_LAYERED, "MainWindow", "Test", WS_EX_TOPMOST | WS_POPUP, xy.x, xy.y, width, height, hTargetWnd, NULL, hInstance, NULL);
    if (!hWnd) return -1;

    // Open a handle of the process with all access rights
    HANDLE hProcess = init_processes("csgo.exe", "client_panorama.dll");
    if (hProcess == NULL)
    {
        cout << "Error: failed to init_processes." << endl;
        return -1;
    }
    // D2D FACTORY & INIT RENDER
    ID2D1Factory* pFactory = Drawer::init_Factory();
    ID2D1HwndRenderTarget* pRT = Drawer::init_RenderTarget(pFactory, hWnd, width, height);
    // Create a pCB (ColorBrush)
    ID2D1SolidColorBrush* pCB = Drawer::init_SolidColorBrush(pRT, D2D1::ColorF(0, 0, 0, 0));

    // Do some weird magic with the window
    SetLayeredWindowAttributes(hWnd, RGB(0, 0, 0), 255, ULW_COLORKEY | LWA_ALPHA);
    
    ShowWindow(hWnd, nCmdShow);

    thread thResizer(WindowResizer, hTargetWnd, hWnd);
    MSG messages = { 0 };
    while (messages.message != WM_QUIT)
    {
        if (PeekMessage(&messages, NULL, 0, 0, PM_REMOVE)) {
            TranslateMessage(&messages);
            DispatchMessage(&messages);
        }
        
        view_matrix_t view_matrix = Reader::RPM<view_matrix_t>(hProcess, baseAddr + dwViewMatrix);
        // Get the local team
        Entity LocalEntity(hProcess, baseAddr + dwEntityList);

        LocalEntity.ReadTeamNum();
        pRT->BeginDraw();
        pRT->Clear(D2D1::ColorF(0, 0, 0, 0));
        for (size_t i = 1; i < 32; i++)
        {
            // Get next entity
            Entity pEntity(hProcess, baseAddr + dwEntityList + (i * 0x10));
            if (pEntity.EntityExist())
            {
                // Read Team Number
                pEntity.ReadTeamNum();
                if (pEntity.sEntity.Team != LocalEntity.sEntity.Team)
                {
                    // Now we can use massive RPM
                    pEntity.ReadEntityDatas();
                    if (pEntity.IsAlive())
                    {
                        // Convert pos vectors to a 2D vector
                        pEntity.WToS(view_matrix);
                        if (pEntity.sEntity.Foot.z > 0.01f)
                        {
                            Drawer::DrawBox(pRT, pCB, pEntity);
                            Drawer::DrawLife(pRT, pCB, pEntity);
                            Drawer::DrawArmor(pRT, pCB, pEntity);
                        }
                    }
                }
            }
            pEntity.~Entity();
        }
        HRESULT hr = pRT->EndDraw();
    }
    Drawer::SafeRelease(&pRT);
    Drawer::SafeRelease(&pCB);
    Drawer::SafeRelease(&pFactory);
}

LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    switch (uMsg)
    {
        case WM_DESTROY:
            stop = false;
            terminate();
            PostQuitMessage(0);
            return 0;
    }
    return DefWindowProc(hwnd, uMsg, wParam, lParam);
}