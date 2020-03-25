#include <iostream>
#include <windows.h>
#include <tlhelp32.h>

DWORD pID = NULL;
DWORD baseAddr = NULL;
DWORD dwEntityList = 0x4D449F4;
DWORD dwLocalPlayer = 0xD30B84;
DWORD m_flFlashDuration = 0xA410;

void get_baseaddr(DWORD pid, char* mname)
{
    MODULEENTRY32 mentry;
    HANDLE snapshot = NULL;
    snapshot = CreateToolhelp32Snapshot(TH32CS_SNAPMODULE, pID);

    if (snapshot == INVALID_HANDLE_VALUE)
    {
        std::cout << "Oh fuck, failed to get snapshot" << std::endl;
        return;
    }

    mentry.dwSize = sizeof(MODULEENTRY32);
    while (Module32Next(snapshot, &mentry))
    {
        if (!strcmp(mname, mentry.szModule))
        {
            std::cout << "Base address of " << mentry.szModule << " is " << std::hex << (unsigned int)mentry.modBaseAddr << std::endl;
            baseAddr = (unsigned int)mentry.modBaseAddr;
            break;
        }
    }
    CloseHandle(snapshot);
}

void get_pid(char *pname)
{
    PROCESSENTRY32 pentry;
    HANDLE snapshot = NULL;
    snapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
    
    if (snapshot == INVALID_HANDLE_VALUE)
    {
        std::cout << "Oh fuck, failed to get snapshot" << std::endl;
        return ;
    }

    pentry.dwSize = sizeof(PROCESSENTRY32);
    while (Process32Next(snapshot, &pentry))
    {
        //std::cout << pentry.szExeFile << std::endl;
        if (!strcmp(pname, pentry.szExeFile))
        {
            std::cout << "We have te process id : " << pentry.th32ProcessID << std::endl;
            pID = pentry.th32ProcessID;
            break ;
        }
    }
    CloseHandle(snapshot);
}

struct vector
{
    float x;
    float y;
    float z;
};

int main()
{
    get_pid((char *)"csgo.exe");
    if (pID != NULL)
    {
        HANDLE process = OpenProcess(PROCESS_ALL_ACCESS, false, pID);
        get_baseaddr(pID, (char*)"client_panorama.dll");
        if (baseAddr != NULL)
        {
            DWORD entityList = NULL;
            
            ReadProcessMemory(process, (LPVOID)(baseAddr + dwEntityList), &entityList, sizeof(DWORD), 0);
            DWORD pEntity = 0;
            struct vector pos;
            while (true)
            {
                std::cout << "-------------------------------------------" << std::endl;
                for (size_t i = 0; i < 32; i++)
                {
                    ReadProcessMemory(process, (LPVOID)(baseAddr + dwEntityList + (i * 0x10)), &pEntity, sizeof(DWORD), 0);
                    ReadProcessMemory(process, (LPVOID)(pEntity + 160), &pos, sizeof(struct vector), 0);
                    if (pEntity != 0)
                        std::cout << pEntity << " x : " << pos.x << " y : " << pos.y << " z : " << pos.z << std::endl;
                }
                std::cout << "-------------------------------------------" << std::endl;
                Sleep(4000);
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
        }
        else
        {
            std::cout << "Failed to find the base address" << std::endl;
        }
    
    }
}