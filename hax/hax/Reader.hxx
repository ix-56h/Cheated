#pragma once
#include "hax.hxx"

namespace Reader {
    template <typename T> T RPM(HANDLE hProcess, SIZE_T address)
    {
        T buffer;

        ReadProcessMemory(hProcess, (LPCVOID)address, &buffer, sizeof(T), 0);

        return (buffer);
    }
}