#pragma once
#include <iostream>
#include <windows.h>
#include "offsets.h"

LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

struct vector
{
    float x, y, z;
};

struct view_matrix_t {
    float matrix[16];
};

struct pEntityVars {
	int		Health;
	int		Team;
	BYTE	Flag;
	//string	Name;
	vector	Pos;
	vector	Head;
};


