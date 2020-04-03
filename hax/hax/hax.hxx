#pragma once
#include <iostream>
#include <windows.h>
#include "offsets.h"

LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
struct vector WorldToScreen(const struct vector pos, struct view_matrix_t matrix);

struct vector
{
    float x, y, z;
};

struct view_matrix_t {
    float matrix[16];
};



