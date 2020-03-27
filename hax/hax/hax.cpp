#include <iostream>
#include <windows.h>
#include <tlhelp32.h>
#include "offsets.h"
#include "hax.h"

DWORD   pID         = NULL;
DWORD   baseAddr    = NULL;
RECT    WBounds;
HPEN    hPen        = CreatePen(PS_SOLID, 1, 0x0000FF);
// Initialization of solid brush for Enemies
HBRUSH  EnemyBrush = CreateSolidBrush(0x0000FF);

// Get screen sizes
int     screenX     = GetSystemMetrics(SM_CXSCREEN);
int     screenY     = GetSystemMetrics(SM_CYSCREEN);


void get_pid(const char *pname)
{
    PROCESSENTRY32 pentry;
    HANDLE snapshot = NULL;
    // Create snapshot of all processes
    snapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
    
    if (snapshot == INVALID_HANDLE_VALUE)
    {
        std::cout << "Oh fuck, failed to get snapshot" << std::endl;
        return ;
    }
    // Init the struct size
    pentry.dwSize = sizeof(PROCESSENTRY32);
    // Check if we have a first element in processes snapshot
    if (Process32First(snapshot, &pentry))
    {
        // iterate on it until we get the process
        while (Process32Next(snapshot, &pentry))
        {
            //std::cout << pentry.szExeFile << std::endl;
            if (!strcmp(pname, pentry.szExeFile))
            {
                std::cout << "We have te process id : " << pentry.th32ProcessID << std::endl;
                // save pID
                pID = pentry.th32ProcessID;
                break;
            }
        }
    }
    CloseHandle(snapshot);
}

void get_baseaddr(DWORD pid, const char* mname)
{
    MODULEENTRY32 mentry;
    HANDLE snapshot = NULL;
    // Create a snapshot of all modules of the process associated to the pID
    snapshot = CreateToolhelp32Snapshot(TH32CS_SNAPMODULE, pID);

    if (snapshot == INVALID_HANDLE_VALUE)
    {
        std::cout << "Oh fuck, failed to get snapshot" << std::endl;
        return;
    }

    // Init the struct size
    mentry.dwSize = sizeof(MODULEENTRY32);
    // Check if we have a first element in modules snapshot
    if (Module32First(snapshot, &mentry))
    {
        // iterate on it until we get the module
        while (Module32Next(snapshot, &mentry))
        {
            if (!strcmp(mname, mentry.szModule))
            {
                std::cout << "Base address of " << mentry.szModule << " is " << std::hex << (unsigned int)mentry.modBaseAddr << std::endl;
                // save the base address
                baseAddr = (unsigned int)mentry.modBaseAddr;
                break;
            }
        }
    }
    CloseHandle(snapshot);
}

struct vector WorldToScreen(const struct vector pos, struct view_matrix_t matrix) {
    struct vector out;
    float _x = matrix.matrix[0] * pos.x + matrix.matrix[1] * pos.y + matrix.matrix[2] * pos.z + matrix.matrix[3];
    float _y = matrix.matrix[4] * pos.x + matrix.matrix[5] * pos.y + matrix.matrix[6] * pos.z + matrix.matrix[7];
    out.z = matrix.matrix[12] * pos.x + matrix.matrix[13] * pos.y + matrix.matrix[14] * pos.z + matrix.matrix[15];

    _x *= 1.f / out.z;
    _y *= 1.f / out.z;

    int width = WBounds.right - WBounds.left;
    int height = WBounds.bottom + WBounds.left;

    out.x = width * .5f;
    out.y = height * .5f;

    out.x += 0.5f * _x * width + 0.5f;
    out.y -= 0.5f * _y * height + 0.5f;

    return out;
}

template <typename T> T RPM(HANDLE hProcess, SIZE_T address)
{
    //omg so good
    T buffer;

    ReadProcessMemory(hProcess, (LPCVOID)address, &buffer, sizeof(T), 0);
    
    return (buffer);
}

void DrawBox(HDC hdc, struct vector body, struct vector head, BYTE flag, int thickness)
{
    float height = body.y - head.y;
    float width = height / 2.4f; // 2.4f is a magic number, idk why ...
    if (flag == 7)
        width += 5.f;
    SelectObject(hdc, GetStockObject(NULL_BRUSH));
    SelectObject(hdc, hPen);
    Rectangle(hdc, body.x - (width / 2), body.y, head.x + (width / 2), head.y);
    DeleteObject(hPen);
}

int main()
{
    // Get de pID of the process
    get_pid((const char *)"csgo.exe");
    // Get the module base address
    get_baseaddr(pID, (const char *)"client_panorama.dll");
    // Open a handle of the process with all access rights
    HANDLE hProcess = OpenProcess(PROCESS_ALL_ACCESS, false, pID);
    // Open a handle to the device context from the top-level handle window (here csgo window)
    HWND hWindow = FindWindowA(NULL, "Counter-Strike: Global Offensive");
    GetClientRect(hWindow, &WBounds);
    HDC hdc = GetDC(hWindow);
    while (true)
    {
        // ????
        view_matrix_t view_matrix = RPM<view_matrix_t>(hProcess, baseAddr + dwViewMatrix);
        // Get the local team
        DWORD pEntity = RPM<DWORD>(hProcess, baseAddr + dwEntityList);
        int localTeam = RPM<int>(hProcess, pEntity + m_iTeamNum);
        //printf("%d\n", localTeam);
        //std::cout << "-------------------------------" << std::endl;
        for (size_t i = 1; i < 32; i++)
        {
            // Get next entity struct
            pEntity = RPM<DWORD>(hProcess, baseAddr + dwEntityList + (i * 0x10));
            if (pEntity)
            {
                // From this entity, get all data we need
                int team = RPM<int>(hProcess, pEntity + m_iTeamNum);
                if (team != localTeam)
                {
                    int health = RPM<int>(hProcess, pEntity + m_iHealth);
                    if (health > 0 && health < 101)
                    {
                        BYTE flag = RPM<BYTE>(hProcess, pEntity + m_fFlags);
                        vector pos = RPM<vector>(hProcess, pEntity + m_vecOrigin);
                        //printf("team = %d ||| x = %.5f ||| y  = %.5f ||| z = %.5f\n", team, pos.x, pos.y, pos.z);
                        // Calculate the position of the head
                        vector head = pos;
                        head.z += 72.f; // 75.f is the physical size of the body in the world, i think ?
                        if (flag == 7) // sitting
                            head.z -= 22.f; // 36.f is the physical size of the body in the world, i think ?
                        // Convert pos vectors to a 2D vector
                        vector screen_body = WorldToScreen(pos, view_matrix);
                        // Same but for the Head pos
                        vector screen_head = WorldToScreen(head, view_matrix);

                        // check on body.z for "body exist", health > 0 for "is alive" and health < 101 for "is not spectator"
                        if (screen_body.z > 0.01f)
                        {
                            DrawBox(hdc, screen_body, screen_head, flag, 1);
                        }
                    }
                }
            }
        }
        //std::cout << "-------------------------------" << std::endl;
    }
}
/* anti flash
if (localp == NULL)
{
    while (localp == NULL)
        ReadProcessMemory(process, (LPVOID)(baseAddr + dwLocalPlayer), &localp, sizeof(DWORD), 0);
}
DWORD flash = 0;
while (true)
{
    ReadProcessMemory(process, (LPVOID)(localp + m_flFlashDuration), &flash, sizeof(DWORD), 0);
    if (flash > 0)
    {
        flash = 0;
        WriteProcessMemory(process, (LPVOID)(localp + m_flFlashDuration), &flash, sizeof(DWORD), 0);
    }
    //std::cout << flash << std::endl;
}
*/