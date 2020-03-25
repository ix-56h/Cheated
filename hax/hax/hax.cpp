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

int main()
{
    get_pid((char *)"csgo.exe");
    if (pID != NULL)
    {
        HANDLE process = OpenProcess(PROCESS_ALL_ACCESS, false, pID);
        get_baseaddr(pID, (char*)"client_panorama.dll");
        if (baseAddr != NULL)
        {
            DWORD localp;
            
            ReadProcessMemory(process, (LPVOID)(baseAddr + dwLocalPlayer), &localp, sizeof(DWORD), 0);
            if (localp == NULL)
            {
                while (localp == NULL)
                    ReadProcessMemory(process, (LPVOID)(baseAddr + dwLocalPlayer), &localp, sizeof(DWORD), 0);
            }
            /* anti flash  
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